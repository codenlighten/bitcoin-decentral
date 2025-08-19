#!/usr/bin/env python3
"""
Bitcoin Decentral Rebranding Script
Systematically renames Bitcoin Core identifiers to Bitcoin Decentral (BTCD)
"""

import os
import re
import sys
from pathlib import Path

# Rebranding mappings
REBRANDING_MAP = {
    # Core project names
    'bitcoin': 'bitcoindecentral',
    'Bitcoin': 'BitcoinDecentral', 
    'BITCOIN': 'BITCOINDECENTRAL',
    'BTC': 'BTCD',
    'btc': 'btcd',
    
    # Network identifiers
    'bitcoin-core': 'bitcoindecentral-core',
    'Bitcoin Core': 'Bitcoin Decentral',
    'bitcoincore': 'bitcoindecentral',
    'BitcoinCore': 'BitcoinDecentral',
    
    # URLs and domains
    'bitcoincore.org': 'bitcoindecentral.org',
    'bitcoin.org': 'bitcoindecentral.org',
    
    # Binary names
    'bitcoind': 'bitcoindecentrald',
    'bitcoin-cli': 'bitcoindecentral-cli',
    'bitcoin-tx': 'bitcoindecentral-tx',
    'bitcoin-wallet': 'bitcoindecentral-wallet',
    'bitcoin-qt': 'bitcoindecentral-qt',
    
    # Configuration files
    'bitcoin.conf': 'bitcoindecentral.conf',
    
    # Network magic numbers (will be updated separately)
    # Address prefixes (will be updated separately)
}

# Files to exclude from rebranding
EXCLUDE_FILES = {
    '.git',
    'bitcoin-core-source',
    '__pycache__',
    '.pytest_cache',
    'rebrand.py',
    'PROGRESS.md',  # Keep our progress tracking intact
}

# File extensions to process
INCLUDE_EXTENSIONS = {
    '.cpp', '.h', '.hpp', '.c',
    '.py', '.sh', '.md', '.txt',
    '.cmake', '.yml', '.yaml',
    '.json', '.xml', '.html'
}

def should_process_file(file_path):
    """Check if file should be processed for rebranding"""
    # Skip excluded directories and files
    for exclude in EXCLUDE_FILES:
        if exclude in str(file_path):
            return False
    
    # Only process files with specific extensions
    return file_path.suffix.lower() in INCLUDE_EXTENSIONS

def rebrand_file_content(file_path):
    """Rebrand content within a file"""
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        
        original_content = content
        
        # Apply rebranding mappings
        for old_term, new_term in REBRANDING_MAP.items():
            # Use word boundaries to avoid partial matches
            pattern = r'\b' + re.escape(old_term) + r'\b'
            content = re.sub(pattern, new_term, content)
        
        # Special case: Update copyright notices
        content = re.sub(
            r'Copyright \(c\) (\d{4}(?:-\d{4})?)\s+The Bitcoin Core developers',
            r'Copyright (c) \1 The Bitcoin Core developers\n// Copyright (c) 2025 The Bitcoin Decentral developers',
            content
        )
        
        # Only write if content changed
        if content != original_content:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            return True
        
        return False
        
    except Exception as e:
        print(f"Error processing {file_path}: {e}")
        return False

def rebrand_filename(file_path):
    """Rebrand filename if needed"""
    old_name = file_path.name
    new_name = old_name
    
    for old_term, new_term in REBRANDING_MAP.items():
        if old_term in old_name:
            new_name = new_name.replace(old_term, new_term)
    
    if new_name != old_name:
        new_path = file_path.parent / new_name
        try:
            file_path.rename(new_path)
            print(f"Renamed: {old_name} -> {new_name}")
            return new_path
        except Exception as e:
            print(f"Error renaming {file_path}: {e}")
            return file_path
    
    return file_path

def main():
    """Main rebranding process"""
    project_root = Path(__file__).parent.parent
    print(f"Starting Bitcoin Decentral rebranding in: {project_root}")
    
    files_processed = 0
    files_changed = 0
    
    # Process all files recursively
    for root, dirs, files in os.walk(project_root):
        # Skip excluded directories
        dirs[:] = [d for d in dirs if d not in EXCLUDE_FILES]
        
        for file in files:
            file_path = Path(root) / file
            
            if not should_process_file(file_path):
                continue
            
            files_processed += 1
            
            # Rebrand file content
            if rebrand_file_content(file_path):
                files_changed += 1
                print(f"Updated content: {file_path.relative_to(project_root)}")
            
            # Rebrand filename
            rebrand_filename(file_path)
    
    print(f"\nRebranding complete!")
    print(f"Files processed: {files_processed}")
    print(f"Files changed: {files_changed}")
    
    print("\nNext steps:")
    print("1. Update network magic numbers in chainparams.cpp")
    print("2. Create new genesis block")
    print("3. Update address prefixes")
    print("4. Test compilation")

if __name__ == "__main__":
    main()
