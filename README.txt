
TODO:
- fix table dump to print full path, no comment
- DONE convert heads to a linear format, for easy saving.
- DONE ' ' fails    
    

WATCH OUT:
- cleanse table
- watch out for table holes
- corner condition: table full, next compile will fail...

+ hexd 
{ $F and '0' +
$39 over <= if 7 + thanx ; }

+--------+------------------------------+-------------+--------------------+
|Instr   | Description                  | signed-ness | Flags              |
+--------+------------------------------+-------------+--------------------+
| JO     | Jump if overflow             |             | OF = 1             |
+--------+------------------------------+-------------+--------------------+
| JNO    | Jump if not overflow         |             | OF = 0             |
+--------+------------------------------+-------------+--------------------+
| JS     | Jump if sign                 |             | SF = 1             |
+--------+------------------------------+-------------+--------------------+
| JNS    | Jump if not sign             |             | SF = 0             |
+--------+------------------------------+-------------+--------------------+
| JE/    | Jump if equal                |             | ZF = 1             |
| JZ     | Jump if zero                 |             |                    |
+--------+------------------------------+-------------+--------------------+
| JNE/   | Jump if not equal            |             | ZF = 0             |
| JNZ    | Jump if not zero             |             |                    |
+--------+------------------------------+-------------+--------------------+
| JP/    | Jump if parity               |             | PF = 1             |
| JPE    | Jump if parity even          |             |                    |
+--------+------------------------------+-------------+--------------------+
| JNP/   | Jump if no parity            |             | PF = 0             |
| JPO    | Jump if parity odd           |             |                    |
+--------+------------------------------+-------------+--------------------+
| JCXZ/  | Jump if CX is zero           |             | CX = 0             |
| JECXZ  | Jump if ECX is zero          |             | ECX = 0            |
+--------+------------------------------+-------------+--------------------+

Then the unsigned ones:

+--------+------------------------------+-------------+--------------------+
|Instr   | Description                  | signed-ness | Flags              |
+--------+------------------------------+-------------+--------------------+
| JB/    | Jump if below                | unsigned    | CF = 1             |
| JNAE/  | Jump if not above or equal   |             |                    |
| JC     | Jump if carry                |             |                    |
+--------+------------------------------+-------------+--------------------+
| JNB/   | Jump if not below            | unsigned    | CF = 0             |
| JAE/   | Jump if above or equal       |             |                    |
| JNC    | Jump if not carry            |             |                    |
+--------+------------------------------+-------------+--------------------+
| JBE/   | Jump if below or equal       | unsigned    | CF = 1 or ZF = 1   |
| JNA    | Jump if not above            |             |                    |
+--------+------------------------------+-------------+--------------------+
| JA/    | Jump if above                | unsigned    | CF = 0 and ZF = 0  |
| JNBE   | Jump if not below or equal   |             |                    |
+--------+------------------------------+-------------+--------------------+

And, finally, the signed ones:

+--------+------------------------------+-------------+--------------------+
|Instr   | Description                  | signed-ness | Flags              |
+--------+------------------------------+-------------+--------------------+
| JL/    | Jump if less                 | signed      | SF <> OF           |
| JNGE   | Jump if not greater or equal |             |                    |
+--------+------------------------------+-------------+--------------------+
| JGE/   | Jump if greater or equal     | signed      | SF = OF            |
| JNL    | Jump if not less             |             |                    |
+--------+------------------------------+-------------+--------------------+
| JLE/   | Jump if less or equal        | signed      | ZF = 1 or SF <> OF |
| JNG    | Jump if not greater          |             |                    |
+--------+------------------------------+-------------+--------------------+
| JG/    | Jump if greater              | signed      | ZF = 0 and SF = OF |
| JNLE   | Jump if not less or equal    |             |                    |
+--------+------------------------------+-------------+--------------------+
