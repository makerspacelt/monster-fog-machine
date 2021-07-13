<?php

if (isset($_GET['f']) && (trim($_GET['f']) != '')) {
   processFunc(explode('?', $_GET['f'])[0]);
} else {
    echo 'no function call specified';
}

function processFunc($func) {
    // emulate ESP32 response lag
    $sTime = rand(50, 200) * 1000;
    usleep($sTime);
    
     if ($func == '/start-manual') {
         do_startManual();
     } else if ($func == '/stop-manual') {
         do_stopManual();
     } else if ($func == '/start-auto') {
         do_startAuto();
     } else if ($func == '/stop-auto') {
         do_stopAuto();
     } else if ($func == '/heater-status') {
         do_heaterStatus();
     } else if ($func == '/liquid-status') {
         do_liquidStatus();
    } else if ($func == '/get-delay-time') {
         do_delayTime();
    } else if ($func == '/get-spray-time') {
         do_sprayTime();
     } else if ($func == '/mode-status') {
         do_modeStatus();
     } else {
         echo 'no function processor found';
     }
}

function do_startManual() {
    // response: true if started, false if not
    echo 'true';
}

function do_stopManual() {
    // response: true if stopped, false if not
    echo 'true';
}

function do_startAuto() {
    // response: true if started, false if not
    echo 'true';
}

function do_stopAuto() {
    // response: true if stopped, false if not
    echo 'true';
}

function do_heaterStatus() {
    // response: true if ready, false if heating
    switch ($_GET['sensor']) {
        case '1': echo 'true'; break;
        case '2': echo 'false'; break;
        case '3': echo 'true'; break;
    }
}

function do_liquidStatus() {
    // response: true if not empty, false if empty
    echo 'true';
}

function do_delayTime() {
    // response: number in minutes
    echo '9';
}

function do_sprayTime() {
    // response: number in seconds
    echo '19';
}

function do_modeStatus() {
    // response: none, manual or auto
    echo 'none';
}

?>