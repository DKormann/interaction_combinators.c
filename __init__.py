"""
tinycombinator - A minimal Python and C implementation of interaction calculus runtime
"""

__version__ = "0.1.0"

from tinycombinator.node import IC, Tag
from tinycombinator.main import execute

__all__ = ["IC", "Tag", "execute"]


