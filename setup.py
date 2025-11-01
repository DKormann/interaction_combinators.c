"""
Setup script for tinycombinator package
"""
from setuptools import setup, find_packages

setup(
    name="tinycombinator",
    version="0.1.0",
    packages=find_packages(),
    python_requires=">=3.10",
    install_requires=[
        "typing-extensions>=4.0",
    ],
    extras_require={
        "dev": [
            "pytest>=7.0",
            "pytest-xdist>=2.5",
        ],
        "test": [
            "pytest>=7.0",
            "pytest-xdist>=2.5",
        ],
    },
    author="Iain Banks",
    description="A minimal Python and C implementation of interaction calculus runtime",
    long_description=open("README.md").read(),
    long_description_content_type="text/markdown",
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Topic :: Software Development :: Interpreters",
    ],
)
