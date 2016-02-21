# Introduction #

Table **corpus** store current corpus for looking up. <br />
Table **candidate** store the candidate phrase which is not include in **corpus**. <br />


# Details #

1. Table **corpus** and **candidate** share the common _id_ as PK. <br />
2. We used to compute crc-64 value of _syllable_ as _crc_ for fast integer index, but this column become **DEPRECATED**, i.e., use _syllable_ as key directly now. <br />
3. We design a new encoding method for transforming from phone to _syllable_ as EncodingTable page. <br />