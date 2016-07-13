#!/usr/bin/perl

use sCode;

print "This program generates data to be put into smart cards\n";
print "Name:"; $nam = <>; chomp $nam; @name = ('s', $nam);

print "Address:"; $adr = <>; chomp $adr; @addr = ('s', $adr);

print "Sex:"; $sx = <>; chomp $sx; @sex = ('s', $sx);

print "Title:"; $tit = <>; chomp $tit; @title = ('s', $tit);

print "Balance:"; $bal = <>; chomp $bal; @balance = ('s', $bal);


#@name = ('s', "Vivek K S");
#@addr = ('s', "24, 13th cross, Swimming Pool Extension, Malleswaram, Bangalore");
#@occupation = ('s', "Software Engineer");
#@sex = ('s', "Male");

%hash=();

$hash{'name'} = \@name;
$hash{'addr'} = \@addr;
$hash{'sex'} = \@sex;
$hash{'title'} = \@title;
$hash{'balance'} = \@balance;

print sCode->encode(\%hash, 2);
