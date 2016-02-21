#What is modified so far

# Introduction #

We attempt to connect libchewing phrasing mechanisms to a backend server for phrase lookup.
So far we've made changes to basic phrasing of sentence breakpoint fragments. Context and sentence level phrasing are in the planning.

# Details #

**Lookup Method**<br>
1. Used libcurl for query connection to the server with HTTP POST message<br>
2. Server responds with a list of phrases in JSON format, which is parsed using yajl<br>
3. Phrases are listed according to the responded frequencies;<br>
<br>
<b>Phrase page for user selection</b><br>
TBD<br>
<br>
<b>Sentence page for user selection</b><br>
TBD<br>
<br>
<b>TODO</b><br>
1. Add frequency report to server<br>
2. Add frequency from server to local<br>
3. Change behavior of tree.c<br>