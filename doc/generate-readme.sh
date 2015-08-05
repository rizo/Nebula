#!/bin/bash

pandoc nebula.md -S -s -t markdown_github -o README.md
pandoc nebula.md -t plain -o README
pandoc nebula.md -S -s -c ../doc/dcpu16-universe.css --toc -o README.html

mv README.md ..
mv README ..
mv README.html ..
