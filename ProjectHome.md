CitC (Chewing in the Cloud) is a project based on the popular open source <br>
Chewing Chinese input method, with extended cloud services for real-time <br>
training and phrase selection.<br>
<br>
--<br>
<br>
Dependencies:<br>
<blockquote>libcurl - 7.21.0 <br>
yajl - 1.0.9</blockquote>

Environment Variables:<br>
<blockquote>CHEWING_SERVER_URL - url for looking up corpus (default: "<a href='http://citc.cse.tw'>http://citc.cse.tw</a>") <br>
CHEWING_USER_FEEDBACK - enable feedback for improving corpus (default: null for enable)</blockquote>

Protocol:<br>
<blockquote>OP=0 : lookup, get available phrases form CHEWING_SERVER_URL  in json format<br>
OP=1 : update, send the phrase selected by user (similar to UpdateFreq) <br>
OP=2 : insert, send the phrase and syllable when user is keying</blockquote>

TODO:<br>
<blockquote>autotools - check dependencies for libcurl and yajl <br>
build basic and full corpus files<br>
strategy for looking up corpus <br>
build a platform for everyone to edit corpus <br>
collaborate with Google suggest and Yahoo Taiwan search suggest <br>
refine protocol <br>
security issue <br>
privacy issue <br>