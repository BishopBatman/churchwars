#!/usr/bin/env python3
"""Simulate combat disconnect scenarios to ensure fights terminate or
continue appropriately."""
import sys

class Player:
    def __init__(self, name):
        self.name = name
        self.fight_array = []
        self.event = "E_FIGHT"
        self.attacking = None

def setup_fight(players):
    for p in players:
        p.fight_array = players

allow_next_shooter_called = False

def allow_next_shooter(play):
    global allow_next_shooter_called
    allow_next_shooter_called = True

def withdraw_from_combat(play):
    global allow_next_shooter_called
    fight_array = play.fight_array
    if not fight_array:
        return
    # Determine if fight ends when this player leaves
    remaining = [p for p in fight_array if p is not play]
    fight_done = len(remaining) <= 1
    # Remove player from array
    play.fight_array.remove(play)
    if not fight_done:
        allow_next_shooter(play)
    else:
        for p in remaining:
            p.fight_array = None
    play.fight_array = None

def client_left_server(play):
    if play.fight_array:
        withdraw_from_combat(play)

def assert_(cond, msg):
    if not cond:
        print(msg, file=sys.stderr)
        sys.exit(1)

def main():
    global allow_next_shooter_called
    # Two-player fight should terminate when one leaves
    a = Player("A")
    b = Player("B")
    setup_fight([a, b])
    client_left_server(a)
    assert_(a.fight_array is None, "A fight array not cleared")
    assert_(b.fight_array is None, "B fight should terminate")
    assert_(not allow_next_shooter_called, "Next shooter should not be called")
    # Multi-player fight should continue after one leaves
    allow_next_shooter_called = False
    a = Player("A")
    b = Player("B")
    c = Player("C")
    setup_fight([a, b, c])
    client_left_server(b)
    assert_(b.fight_array is None, "B fight array not cleared")
    assert_(a.fight_array == [a, c] and c.fight_array == [a, c],
            "Remaining players should keep fighting")
    assert_(allow_next_shooter_called, "Next shooter was not allowed")
    sys.exit(0)

if __name__ == "__main__":
    main()
