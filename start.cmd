@echo off

git checkout master
git pull
git submodule update --remote --init --recursive --force -- "third_party/basictypes"
git submodule update --remote --init --recursive --force -- "third_party/dynamic_library"