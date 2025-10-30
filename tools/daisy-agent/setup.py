from setuptools import setup, find_packages

setup(
    name="daisy-agent",
    version="0.1.0",
    description="Session orchestration agent for Daisy Seed development",
    author="Chris Brandt",
    author_email="chris@futr.tv",
    packages=find_packages(where="src"),
    package_dir={"": "src"},
    install_requires=[
        "click>=8.3.0",
        "PyYAML>=6.0.3",
        "rich>=14.2.0",
        "pyperclip>=1.11.0",
    ],
    entry_points={
        "console_scripts": [
            "daisy-agent=daisy_agent.cli:cli",
        ],
    },
    python_requires=">=3.10",
)