CONTENTS OF THIS FILE
---------------------

 * Introduction
 * Burrows–Wheeler transform (BWT)
 * Suffix Array
 * Encoding
 * Backward Search
 * Clarification


INTRODUCTION
------------

This project is going to apply an extended Burrows-Wheeler transform
scheme on given strings by recording the position of each delimiters
(two delimiters separate one record in between).

 * Take string, "ab$abb$acd$", for example, there are three delimiters
   (delimiters would be recoginised beforehand) representing three 
   records, including, "ab", "abb" and "acd".


BWT
---

BWT is a way of manipualting strings by rearranging them in order to 
cluster most of the similiar characters, after which compressing strings
that have repeated characters by, such as, run-length encoding.
One of the simplest approach to construct a BWT is to list all rotations
for the given string and sort them by lexicalgraphical order. Finally, 
the last column would be the result of BWT.
However, BWT usually works on large amounts of data, that given string
can be up to 50M or more long. Storing all the rotations would cost a
lost in running memory. Not mention sorting them which takes even more
time to be finished.


SUFFIX ARRAY
------------

In this case, constructing a suffix array and achieving BWT by that
cannot be a more aprreciable option. Suffix array can be obtainable by
the enumeration of the string moving forwards every time. And after
sorting them, BWT is going to be the letter before first letter in
suffix array.

  * BEFORE 		i		AFTER 		i 			BWT
    banana$		1		$			7			a
    anana$		2		a$			6			n
    nana$		3		ana$		4			n
    ana$		4		anana$		2			b
    na$			5		banana$		1			$
    a$			6		na$			5			a
    $			7		nana$		3			a
    

ENCODING
--------

Similar to above, the first step to do BWT is to construct the suffix
array. One thing different though, applying extended BWT scheme with
positional delimiter information is one important key to implement
backward search. So that, within the encoding phase, an auxiliary file
containing the position of each delimiter in ascending order is necessary.
Say, the sooner the delimiter comes, the lower weight it owns.

  * BEFORE 		i		AFTER 		i 			BWT
    ab$abb$c$	1		$abb$c$		3			b
    b$abb$c$	2		$c$			7			b
    $abb$c$		3		$			9			c
    abb$c$		4		ab$abb$c$	1			$_3
    bb$c$		5		abb$c$		4			$_1
    b$c$		6		b$abb$c$	2			a
    $c$			7		b$c$		6			b
    c$			8		bb$c$		5			a
    $			9		c$			8			$_2

In such case, auxiliary file would have three integers which are 5, 9 and
4 representing the delimiter in offset 5 has the smallest weight, after
which is offset 9 and offset 4. Of course, that change (different weights
of same delimiter but in different positions) needs apply to the sorting
algorithm to maintain the consistency.


BACKWARD SEARCH
---------------

The reason why backward seach is a must in this scheme rather than some
other search algorithm is that within a large amount of data, backward
seach, one method that does not focus on the string combination itself but
on the pattern, is much quicker and more efficient. This algorithm starts
from the last letter in query string in the first column (accessible by C
table) within the range of "FIRST" and "LAST" (accessible by occurace
table) and keeps mapping to the last column to find the previous letter
until found the whole "pattern".


CLARIFICATION
-------------

 * Unicode set: any visible ASCII alphabet (from 32 to 126), tab and
   newline
 * Delimiter set: all unicode set except for tab
 * Empty reocrd may exist, such as ab$$abb$, which means that the second
   record is empty