<?php

var_dump(emicro_test2());


$application = EMicro\Application::getInstance();


$server = $application->hello();


var_dump($server);