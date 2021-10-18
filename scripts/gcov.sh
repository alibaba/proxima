#!/bin/bash

project_name=proxima-be
gcov_tool=gcov
zip_html=false
output_name=html

script_dir=$(cd "$(dirname "$0")"; pwd)
source_base=$(dirname "$script_dir")
filter_list="'*/tests/*' '*/thirdparty/*' '*/deps/*' '*/proto/*' '*/external/*'"

while getopts t:p:o:z option; do
  case "$option" in
  t)
    gcov_tool=$OPTARG;;
  p)
    project_name=$OPTARG;;
  o)
    output_name=$OPTARG;;
  z)
    zip_html=true;;
  esac
done

# Process sources
lcov -c -b "$source_base" -d . -o $project_name.lcov.info --gcov-tool=$gcov_tool --no-external || exit 1
eval $(echo lcov -r $project_name.lcov.info -o $project_name-filtered.lcov.info $filter_list) || exit 1

# Gather HTML files
genhtml -t "$project_name" -o $output_name $project_name-filtered.lcov.info || exit 1
rm -rf *.lcov.info

# Zip HTML files
if $zip_html ; then
  zip -r $output_name.zip $output_name/
fi
