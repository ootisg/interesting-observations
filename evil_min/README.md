This is built on the observation that if you have two numbers ```(A, B)```, and you subtract ```A``` from ```B```, the result will be negative if ```B > A```. This can be used to create a branchless ```min()``` or ```max()``` function.  

Negative integers on basically all modern systems are represented using two's compliment. A useful detail of this is that if an int is negative, the leftmost bit will be set:
```
1xxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
```
This can be combined with a quirk of the arithmetic shift right operation. On most platforms, the ```>>``` operator will fill the "empty" bits on the left with the sign bit. This results in the following property for signed 32-bit integers:
- Shifting a positive number right by 31 ```(x >> 31)``` will always evaluate to ```0```
- Shifting a negative number right by 31 will always evaluate to ```0xFFFFFFFF``` (all bits are 1; as a numeric value, this is actually ```-1```)  

This is all we need actually. The process is as follows:  

1) Subtract ```A``` from ```B```, then shift right by 31. Save the result in a temp variable (```tmp```)
2) Bitwise AND ```tmp``` with ```A```, and the bitwise inverse of ```tmp``` with ```B```
3) Bitwise OR the results of the two bitwise ands to get the min of ```A``` and ```B```  

Here's what this looks like when ```B > A```:
```
(note: a and b represent the bits of A and B, respectively. x represents an unknown bit, e.g. a bit that could be any value for this to work).
tmp = (A - B) >> 31; 
(A - B = 1xxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx,
 tmp = 11111111 11111111 11111111 11111111)

  A     aaaaaaaa aaaaaaaa aaaaaaaa aaaaaaaa
& tmp   11111111 11111111 11111111 11111111
-------------------------------------------------
= A     aaaaaaaa aaaaaaaa aaaaaaaa aaaaaaaa

  B     bbbbbbbb bbbbbbbb bbbbbbbb bbbbbbbb
& ~tmp  00000000 00000000 00000000 00000000
-------------------------------------------------
= 0     00000000 00000000 00000000 00000000

(A & tmp)    aaaaaaaa aaaaaaaa aaaaaaaa aaaaaaaa
| (B & tmp)  00000000 00000000 00000000 00000000
-------------------------------------------------
= A          aaaaaaaa aaaaaaaa aaaaaaaa aaaaaaaa
```
And when ```A > B```:
```
tmp = (A - B) >> 31;
(A - B = 0xxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx,
 tmp = 00000000 00000000 00000000 00000000)

  A     aaaaaaaa aaaaaaaa aaaaaaaa aaaaaaaa
& tmp   00000000 00000000 00000000 00000000
-------------------------------------------------
= 0     00000000 00000000 00000000 00000000

  B     bbbbbbbb bbbbbbbb bbbbbbbb bbbbbbbb
& ~tmp  11111111 11111111 11111111 11111111
-------------------------------------------------
= B     bbbbbbbb bbbbbbbb bbbbbbbb bbbbbbbb

(A & tmp)    00000000 00000000 00000000 00000000
| (B & tmp)  bbbbbbbb bbbbbbbb bbbbbbbb bbbbbbbb
-------------------------------------------------
= B          bbbbbbbb bbbbbbbb bbbbbbbb bbbbbbbb
```
  
Note that in both cases, the result is the smaller of the two values.  
  
The behavior of arithematic shifts copying the sign bit over the whole number here actually isn't necessary; there's another trick that can be used to get around it. To illustrate this, an example will be provide below with the logical shift right operator (usually written as ```>>>```). This will fill the empty bits in the results with zeroes instead of copying the sign bit.
```
tmp = 0 - ((A - B) >>> 31);
When B > A:
    A - B = 1xxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
    (A - B) >>> 31 = 00000000 00000000 00000000 00000001 (= 1)

        0   00000000 00000000 00000000 00000000
     -  1   00000000 00000000 00000000 00000001
    ----------------------------------------------
       -1   11111111 11111111 11111111 11111111  (this happens because of underflow)
When A > B:
    A - B = 0xxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
    (A - B) >>> 31 = 00000000 00000000 00000000 00000000 (= 0)

        0   00000000 00000000 00000000 00000000
     -  0   00000000 00000000 00000000 00000000
    ----------------------------------------------
        0   00000000 00000000 00000000 00000000
```
The rest is the same as before because we now have the correct value in ```tmp```.

I did something a bit different for the ```max()``` function in min.h. Here's a short overview:

It's a goofy way to use the ```min()``` function to compute ```max()```, and it works perfectly well regardless. It exploits some specific properties of bitwise xor:  
- Any number XOR'd with itself is 0. For instance,
```A XOR A = 0```
- Additionally, any number XOR'd with 0 is unchanged;
```A XOR 0 = A```  

Strangely, these properties are both commutative and associative. This allows you to do some really wacky things! For instance, no matter what values you use for ```A```, ```B```, ```C```, and ```D```,
```
A XOR B XOR D XOR C XOR D XOR D XOR A XOR C XOR B = D
```
because
```
  A XOR B XOR D XOR C XOR D XOR D XOR A XOR C XOR B
= A XOR A XOR B XOR B XOR C XOR C XOR D XOR D XOR D
= (A XOR A) XOR (B XOR B) XOR (C XOR C) XOR (D XOR D) XOR D
= 0 XOR 0 XOR 0 XOR 0 XOR D
= D
```
There's also a very famous example for swapping two variables without using a temporary variable:
```
void swap(int* a, int* b) {
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}
```
That one works as follows:
```
A'  = A XOR B
B'  = B XOR A'  = B XOR A XOR B
A'' = A' XOR B' = A XOR B XOR B XOR A XOR B
```
The relevant values are ```B'``` and ```A''```, since they are the final states of ```A``` and ```B``` respectively--
```
B'
= B XOR A XOR B
= B XOR B XOR A
= (B XOR B) XOR A
= 0 XOR A
= A

A''
= A XOR B XOR B XOR A XOR B
= A XOR A XOR B XOR B XOR B
= (A XOR A) XOR (B XOR B) XOR B
= 0 XOR 0 XOR B
= B
```
The relevant observation for ```max()``` is that if you ```XOR``` either ```A``` or ```B``` with the value ```(A XOR B)```, you will end up with the opposite one. Since ```max(A, B)``` should return the opposite of ```min(A, B)```, this is a perfect opprotunity to use this trick!  

When ```B > A```:
```
  max(A, B)
= A XOR B XOR min(A, B) 
= A XOR B XOR A
= (A XOR A) XOR B
= B
```
And when ```A > B```:
```
  max(A, B)
= A XOR B XOR min(A, B) 
= A XOR B XOR B
= (B XOR B) XOR A
= A
```