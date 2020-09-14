#!/bin/sh
# Ref: Kernel Hackers' Guide to git
#  http://linux.yyz.us/git-howto.html , 
#  http://pradheepshrinivasan.github.io/2011/12/29/cloning-linux-next-tree-i-wanted-to-do/ 
#  http://pradheepshrinivasan.github.io/2015/08/05/Tracking-current-in-linux-next/
name=$(basename $0)

echo "${name}: !NOTE! You must specify whether you want to clone the:
 'regular' kernel src tree (by setting REGULAR_TREE=1 and LINUX_NEXT_TREE=0  in this script), 
-or-
 'linux-next' kernel src tree (by setting REGULAR_TREE=0 and LINUX_NEXT_TREE=1 in this script).

Press [Enter] to continue, or ^C to exit ...
 [Enter] will cause the 'regular' kernel to be git clone'd
"
read x

REGULAR_TREE=1
LINUX_NEXT_TREE=0  # linux-next: working with the bleeding edge?

if [ ${REGULAR_TREE} -eq 1 -a ${LINUX_NEXT_TREE} -eq 1 ] ; then
  echo "${name}: Both 'regular' and 'linux-next' can't be cloned, choose one of them pl.."
  exit 1
fi
if [ ${REGULAR_TREE} -eq 0 -a ${LINUX_NEXT_TREE} -eq 1 ] ; then
  [ $# -ne 1 ] && {
     echo "Working with linux-next:"
	 echo "Usage: ${name} new-branch-to-work-under"
     exit 1
  }
  NEW_BRANCH=$1
fi

[ -d .git ] && {
 echo "${name}: whoops, this dir already has a .git working folder. Continue here or abort?
  [Enter] to continue, ^C to abort"
  read x
}

[ ${REGULAR_TREE} -eq 1 ] && {
  GITURL=https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git
  echo "${name}: cloning 'regular' linux kernel now ... (this can take a while)..."
  echo "Running: time git clone ${GITURL}"
  time git clone ${GITURL}
}
# For 'regular': to update to latest:
# git pull ${GITURL}
# or just
# git pull

[ ${LINUX_NEXT_TREE} -eq 1 ] && {
  GITURL=git://git.kernel.org/pub/scm/linux/kernel/git/next/linux-next.git
  echo "${name}: cloning latest 'linux-next' linux kernel now ... (this can take a while)..."
  echo "Running: time git clone ${GITURL}"
  time git clone ${GITURL}
  cd linux-next || exit 1
  echo " Running: git checkout master"
  git checkout master
  echo " Running: git remote update"
  git remote update
  LATEST_TAG=$(git tag -l next-* | tail -n1)
  echo " Latest tag: ${LATEST_TAG}"
  echo " Running: git checkout -b ${NEW_BRANCH} ${LATEST_TAG}"
  git checkout -b ${NEW_BRANCH} ${LATEST_TAG}
}

echo "Done"
# Could use 'gitk' to see git repos in a GUI
# http://gitk.sourceforge.net/
# http://lostechies.com/joshuaflanagan/2010/09/03/use-gitk-to-understand-git/
