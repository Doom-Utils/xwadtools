#!/bin/sh

# wad file created with 
# slige -seed 2224 -rooms 3 -doom2 -MAP01 -outfile slige.wad

#set -v
#set -x

#input
in_wad="slige.wad"
in_map="MAP01"

# doom, doom2, hexen
in_game="doom2"

tmp_dir="/tmp"

work1_wad="$tmp_dir/work1.wad"
work2_wad="$tmp_dir/work2.wad"
work_txt="$tmp_dir/work_txt_wad"

# $1 input
# $2 output
strip_nodes()
{
  strip_text="$tmp_dir/strip_text"
  wadldc -g $in_game $in_map $1 $strip_text >/dev/null
  wadlc $strip_text $2 >/dev/null
  rm $strip_text
}

# does not check nodes...
# $1 wad file
check_binary_wad()
{
  check_text_file1="$tmp_dir/check_text1"
  check_text_file2="$tmp_dir/check_text2"
  check_tmp_wad="$tmp_dir/check_tmp.wad"
# decompile wad, compile it, decompile again...
  wadldc -g $in_game $in_map $1 $check_text_file1 >/dev/null
  wadlc $check_text_file1 $check_tmp_wad >/dev/null
  wadldc -g $in_game $in_map $check_tmp_wad $check_text_file2 >/dev/null
# compare the text files ... could also use diff...
  cmp $check_text_file1 $check_text_file2
  if test $? -ne 0 
  then
    echo "error: binary wad check failed (wadldc/wadlc)"
    exit
  fi
  echo "binary wad check passed"
  rm $check_text_file1
  rm $check_text_file2
  rm $check_tmp_wad
}

# assumes that the wad has nodes
# $1 wad file
check_binary_wad_with_xdoom()
{
  xdoom -timedemo slige -file $1 >/dev/null
  if test $? -ne 0
  then
    echo "error: xdoom failed to play $1"
    exit
  fi
  echo "playing wad with xdoom passed"
}

check_binary_wad_with_idbsp()
{
  check_text_file1="$tmp_dir/check_text1.dwd"
  check_text_file2="$tmp_dir/check_text2.dwd"
  check_tmp_wad="$tmp_dir/check_tmp.wad"
# decompile wad, compile it, decompile again...
  waddwd $1 $check_text_file1 >/dev/null
  idbsp $check_text_file1 $check_tmp_wad >/dev/null
  waddwd $check_tmp_wad $check_text_file2 >/dev/null
# compare the text files ... could also use diff...
  cmp $check_text_file1 $check_text_file2
  if test $? -ne 0 
  then
    echo "error: binary wad check failed (waddwd/idbsp)"
    exit
  fi
  echo "binary wad check passed"
  rm $check_text_file1
  rm $check_text_file2
  rm $check_tmp_wad
}

# check input file
echo check input file $in_wad using wadlc
cp -f $in_wad $work1_wad
check_binary_wad $work1_wad
echo check input file $in_wad using idbsp
cp -f $in_wad $work1_wad
check_binary_wad_with_idbsp $work1_wad

# check bsp
echo check bsp with $in_wad
cp -f $in_wad $work1_wad
bsp $work1_wad -o $work2_wad >/dev/null 2>/dev/null
check_binary_wad $work2_wad
check_binary_wad_with_xdoom $work2_wad

# check bsp again
echo check bsp again with $in_wad
strip_nodes $in_wad $work1_wad
bsp $work1_wad -o $work2_wad >/dev/null 2>/dev/null
check_binary_wad $work2_wad
check_binary_wad_with_xdoom $work2_wad

# check waddwd/idbsp
echo check waddwd and idbsp with $in_wad
waddwd $in_wad $work_txt.dwd >/dev/null
idbsp $work_txt.dwd $work1_wad >/dev/null
rm $work_txt.dwd
check_binary_wad $work1_wad
check_binary_wad_with_xdoom $work1_wad

# check wadldc/idbsp
echo check wadldc and idbsp with $in_wad
wadldc -g $in_game $in_map $in_wad $work_txt.map
idbsp $work_txt.map $work1_wad >/dev/null
rm $work_txt.map
check_binary_wad $work1_wad
check_binary_wad_with_xdoom $work1_wad

# check warm
echo check warm
strip_nodes $in_wad $work1_wad
warm $work1_wad $work2_wad >/dev/null
check_binary_wad $work2_wad
check_binary_wad_with_xdoom $work2_wad

# check wreject
echo check wreject with bsp with $in_wad
strip_nodes $in_wad $work1_wad
bsp $work1_wad -o $work2_wad >/dev/null 2>/dev/null
wreject $work2_wad $in_map 600 >/dev/null 2>/dev/null
check_binary_wad $work2_wad
check_binary_wad_with_xdoom $work2_wad

# clean up
rm -f $work1_wad $work2_wad
