goto() {
  if [ "$#" -eq 1 ] ; then
    # change directory if gotobin is successful
    gotobin $1
    if [ $? -eq 1 ] ; then
      cd $(gotobin $@)
    fi
  else
    # send arguments to gotobin
    gotobin $@
  fi
}
