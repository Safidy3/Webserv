<?php

    set_time_limit(5);
    header('Content-Type: text/plain');
    echo "Start loop...\n";
    $start = time();
    while (true) {
        sleep(1);
        echo "tick\n";
        flush();
        if (time() - $start > 4)
            break;
    }
    echo "Done\n";
    
?>
