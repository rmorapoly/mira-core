#!/bin/bash

cd ../

# Delete generated files
rm -f Makefile
rm -f moc_*.*
rm -f qmake_*.*
rm -f qmlcache_*.*
rm -f qrc_*.*
rm -f src_*.*
rm -f *.o
rm -f *.moc
rm -f target_*.sh

# Delete build folders
rm -rf release/
rm -rf debug/
rm -rf profile/

echo "Clean completed."
