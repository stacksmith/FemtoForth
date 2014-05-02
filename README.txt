
TODO:
- fix table dump to print full path, no comment
- convert heads to a linear format, for easy saving.
    -next,dad,child,type head pointers


WATCH OUT:
- cleanse table
- watch out for table holes
- corner condition: table full, next compile will fail...

+ hexd 
{ $F and '0' +
$39 over <= if 7 + thanx ; }
