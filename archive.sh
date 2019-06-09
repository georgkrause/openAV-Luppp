#!/bin/bash

./version.sh
git archive HEAD --prefix=loopp-beta/ --format=zip > looppGit_`date +"%d-%m-%Y"`_`git log --pretty=format:"%h" | head -n1`.zip
