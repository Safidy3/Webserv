<?php
// script qui termine immédiatement avec un code de sortie non nul
http_response_code(200);
echo "Content-Type: text/plain\r\n\r\n";
// exit with non-zero: use exit() to return to interpreter — but when using php-cgi the process exit code will be non-zero
exit(1);
