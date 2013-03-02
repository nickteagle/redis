<?php

require_once __DIR__ . '/redis_ssi.module';
$session_name = session_name();
if (isset($_COOKIE[$session_name]) && !redis_ssi_client(TRUE, TRUE)->get('session:settings:' . $_COOKIE[$session_name])) {
  unset($_COOKIE[$session_name]);
}
require_once DRUPAL_ROOT . '/' . variable_get('session_inc_real', 'includes/session.inc');
