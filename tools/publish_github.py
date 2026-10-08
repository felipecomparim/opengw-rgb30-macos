#!/usr/bin/env python3
"""Publish the prepared source commit and release archives using GitHub CLI.

Run after gh auth login. No credentials are stored in this script or repository.
"""
from pathlib import Path
import argparse
import os
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--git-dir', type=Path, help='Optional external Git metadata directory')
parser.add_argument('--owner', default='felipecomparim')
parser.add_argument('--repo', default='opengw-rgb30-macos')
args = parser.parse_args()
git_dir = args.git_dir.resolve() if args.git_dir else root / '.git'
repository = f'{args.owner}/{args.repo}'

def run(*command):
    return subprocess.run(command, cwd=root, check=True, text=True, capture_output=True).stdout.strip()

def git(*command):
    return run('git', f'--git-dir={git_dir}', f'--work-tree={root}', *command)

try:
    login = run('gh', 'api', 'user', '--jq', '.login')
    if login != args.owner:
        raise RuntimeError(f'Authenticated as {login}, expected {args.owner}.')
    if not git_dir.exists():
        raise RuntimeError('Git metadata not found. Initialize and commit the project first.')
    if git('status', '--porcelain'):
        raise RuntimeError('Commit the remaining changes before publishing.')
    assets = [root / 'dist' / name for name in (
        'OpenGW-macOS-AppleSilicon.zip', 'OpenGW-RGB30-ROCKNIX.zip', 'SHA256SUMS')]
    if not all(p.is_file() for p in assets):
        raise RuntimeError('Build both release ZIPs and SHA256SUMS first.')
    existing = subprocess.run(['gh', 'repo', 'view', repository, '--json', 'nameWithOwner'],
        cwd=root, capture_output=True, text=True)
    if existing.returncode == 0:
        raise RuntimeError(f'{repository} already exists; review it before pushing. No existing history was changed.')
    run('gh', 'repo', 'create', repository, '--public', '--description',
        'OpenGW r42 ports for Powkiddy RGB30 / ROCKNIX and macOS Apple Silicon (M4).')
    url = f'https://github.com/{repository}.git'
    remotes = git('remote').splitlines()
    if 'origin' not in remotes:
        git('remote', 'add', 'origin', url)
    elif git('remote', 'get-url', 'origin') != url:
        raise RuntimeError('Origin points to a different repository. No push was attempted.')
    run('git', f'--git-dir={git_dir}', f'--work-tree={root}', '-c', 'credential.helper=',
        '-c', 'credential.helper=!gh auth git-credential', 'push', '-u', 'origin', 'main')
    run('gh', 'release', 'create', 'v1.0.0', *(str(p) for p in assets), '--repo', repository,
        '--target', 'main', '--title', 'OpenGW — RGB30 / ROCKNIX + macOS Apple Silicon',
        '--notes-file', str(root / 'docs/RELEASE-NOTES.md'))
    print(f'Published source: https://github.com/{repository}')
    print(f'Published downloads: https://github.com/{repository}/releases/tag/v1.0.0')
except (RuntimeError, subprocess.CalledProcessError) as error:
    print(str(error), file=sys.stderr)
    if isinstance(error, subprocess.CalledProcessError) and error.stderr:
        print(error.stderr.strip(), file=sys.stderr)
    sys.exit(1)
