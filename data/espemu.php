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
     } else if ($func == '/mode-status') {
         do_modeStatus();
     } else {
         echo 'no function processor found';
     }
}

function do_startManual() {
    
}

function do_stopManual() {
    
}

function do_startAuto() {
    // response: true if started, false if not
    $sensor = explode('?', $_GET['f']);
    if (count($sensor) > 1) {
        $val = explode('=', trim($sensor[1]));
        if ((count($val) > 1) && ($val[1] != '')) {
            echo (rand(0, 1) == 0 ? 'false' : 'true');
        } else {
            echo 'no sensor value';
        }
    } else {
        echo 'no sensor param';
    }
}

function do_stopAuto() {
    
}

function do_heaterStatus() {
    // response: true if ready, false if heating
    $params = explode('?', $_GET['f']);
    if (count($params) > 1) {
        $sensorVal = explode('=', trim($params[1]));
        if ((count($sensorVal) > 1) && ($sensorVal[1] != '')) {
            echo (rand(0, 1) == 0 ? 'false' : 'true');
        } else {
            echo 'no sensor value';
        }
    } else {
        echo 'no sensor param';
    }
}

function do_liquidStatus() {
    // response: true if not empty, false if empty
    echo 'true';
}

function do_modeStatus() {
    // response: none, manual or auto
    echo 'auto';
}

?>