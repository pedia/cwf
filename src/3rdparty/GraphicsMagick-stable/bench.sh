  #!/usr/bin/env bash
  #
  # Measure the performance between two 'convert' commands by
  # executing a subcommand through many iterations and seeing
  # the total time that it takes.
  #
  # Written by Bob Friesenhahn, October 2008
  #

  # GraphicsMagick
#  convert1='/usr/local/bin/gm convert'
convert1='utilities/gm'
  #convert1='/c/Program\ Files/GraphicsMagick-1.3.8-Q16/gm.exe convert'

  # ImageMagick
#convert2='/usr/local/bin/convert'
convert2='../ImageMagick-stable/utilities/convert'
  #convert2='/c/Program\ Files/ImageMagick-6.5.9-Q16/convert.exe'

  # Input file specification
  input_image='-size 1440x900 tile:input.pnm'

  # Ouput file specification
  output_image="jpg:/dev/null"

  # Should not need to change any of the rest
  typeset -i iterations=2
  echo "Convert-1:   ${convert1}"
  echo "Version:     `eval "${convert1}" -version | head -1`"
  echo "Convert-2:   ${convert2}"
  echo "Version:     `eval "${convert2}" -version | head -1`"
  echo "Date:        `date`"
  echo "Host:        `uname -n`"
  echo "OS:          `uname -s`"
  echo "Release:     `uname -r`"
  echo "Arch:        `uname -p`"
  echo "Input File:  ${input_image}"
  echo "Output File: ${output_image}"
  echo "Threads:     ${OMP_NUM_THREADS:-1}"
  echo "Iterations:  ${iterations}"
  echo "========================================================================================"
  echo
typeset -i count=0 i=0
cat commands.txt | while read subcommand
do
    echo ${subcommand}
  
    command1="${convert1} ${subcommand} ${input_image} ${output_image}"
    i=0
    count=$iterations
    time while ((i < count))
    do
      echo $command1
      # eval "${command1}"
      let i=i+1
    done
    sleep 1
  
    command2="${convert2} ${input_image} ${subcommand} ${output_image}"
    i=0
    count=$iterations
    time while ((i < count))
    do
      echo $command2
      # eval "${command2}"
      let i=i+1
    done
  
    echo
    sleep 1
  done 2>&1