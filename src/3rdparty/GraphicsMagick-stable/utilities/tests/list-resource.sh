#!/bin/sh
# Copyright (C) 2009 GraphicsMagick Group

set -e # Exit on any error
. ${srcdir}/utilities/tests/common.sh

${GM} convert -list resource
${GM} mogrify -list resource
