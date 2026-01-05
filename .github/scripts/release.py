#!/usr/bin/env python3
import argparse
import os
import sys

VERSION_FILE = 'VERSION'

def get_current_version():
    try:
        with open(VERSION_FILE, 'r') as f:
            return f.read().strip()
    except FileNotFoundError:
        print(f"Error: {VERSION_FILE} not found.")
        sys.exit(1)

def write_version(version):
    with open(VERSION_FILE, 'w') as f:
        f.write(version)
    print(f"Updated {VERSION_FILE} to {version}")

def bump_version(current_version, bump_type):
    major, minor, patch = map(int, current_version.split('.'))
    
    if bump_type == 'major':
        major += 1
        minor = 0
        patch = 0
    elif bump_type == 'minor':
        minor += 1
        patch = 0
    elif bump_type == 'patch':
        patch += 1
    
    return f"{major}.{minor}.{patch}"

def is_safe_to_reset():
    # Check if the current working directory or git root is 'github_scaffolding' or 'moz-os'
    # This is a safety measure to prevent accidental resets in other repos
    cwd = os.getcwd()
    # Allowing moz-os as well
    return os.path.basename(cwd) in ['github_scaffolding', 'moz-os']

def main():
    parser = argparse.ArgumentParser(description='Manage repository versioning.')
    parser.add_argument('--type', choices=['major', 'minor', 'patch', 'reset'], required=True,
                        help='Type of release or action to perform')
    parser.add_argument('--dry-run', action='store_true', help='Do not modify files')
    
    args = parser.parse_args()
    
    current_version = get_current_version()
    
    if args.type == 'reset':
        if not is_safe_to_reset():
            print("Error: Reset command is only allowed in the 'github_scaffolding' or 'moz-os' repository.")
            sys.exit(1)
        
        new_version = '0.0.0'
        print("Resetting version to 0.0.0...")
    else:
        new_version = bump_version(current_version, args.type)
        print(f"Bumping version from {current_version} to {new_version}")
    
    if not args.dry_run:
        write_version(new_version)
        # Output for GitHub Actions
        if os.getenv('GITHUB_OUTPUT'):
            with open(os.getenv('GITHUB_OUTPUT'), 'a') as fh:
                fh.write(f"version={new_version}\n")

if __name__ == '__main__':
    main()
