#! /bin/sh

if [ $# == 0 ]; then
    echo "Usage: $0 <version>"
    exit 1
fi
newversion=$1
major=`echo $newversion|sed -ne '/^[0-9]*\./{s/\([0-9]*\)\.[0-9]*\.[0-9]*.*/\1/;p}'`
minor=`echo $newversion|sed -ne '/^[0-9]*\.[0-9]*\./{s/[0-9]*\.\([0-9]*\)\.[0-9]*.*/\1/;p}'`
mod=`echo $newversion|sed -ne '/^[0-9]*\.[0-9]*\.[0-9]*$/{s/[0-9]*\.[0-9]*\.\([0-9]*\)/\1/;p}'`
if [ "x$major" = "x" -o "x$minor" = "x" -o "x$mod" = "x" ]; then
    echo "Invalid version number: $newversion" 1>&2
    exit 1
fi
if [ "x`git status -s -uno`" != "x" ]; then
    echo 'uncommited changes in repository' 1>&2
    exit 1
fi
version=`sed -ne '/^AC_INIT/{s/^AC_INIT(.*,[ \t]*\([0-9\.].*\),[ \t]*.*)/\1/;p}' configure.ac`

tmpf=`mktemp liblangtag-NEWS.XXXXXXXX`
header="$version -> $newversion"
n=`echo $header|wc -c`
i=`expr $n + 2`
echo $header >> $tmpf
until [ $i -eq 0 ]; do
    echo -n "=" >> $tmpf
    i=`expr $i - 1`
done
echo >> $tmpf
git log --pretty=short $version.. | git shortlog >> $tmpf
cat NEWS >> $tmpf

mv $tmpf NEWS

sed -i configure.ac -e "/^AC_INIT(/s/$version/$newversion/"

$test git commit -m "Bump the version to $newversion" \
    configure.ac \
    NEWS
$test git tag -s -m "Version $newversion" $newversion
