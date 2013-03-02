<?php

/**
 * Implements hook_drush_command().
 */
function redis_ssi_drush_command() {
  $items['run-ssi-queue'] = array(
    'description' => dt('Consumes items from the queue.'),
    'aliases' => array('redis', 'ssi', 'redis-ssi', 'regen', 'rsrq'),
    'options' => array(
      'timeout' => dt('How long this function should run'),
    ),
  );
  return $items;
}

/**
 * Implements hook_drush_COMMAND().
 */
function drush_redis_ssi_run_ssi_queue() {
  // It costs little to get these simple and fast pages done all the time.
  foreach (array('site_404', 'site_403') as $key) {
    if ($path = drupal_get_normal_path(variable_get($key, ''))) {
      redis_ssi_instantly_regenerate_page($path);
    }
  }
  $timeout = drush_get_option('timeout', 60);
  $queue = redis_ssi_queue_get();
  $_SERVER['HTTP_USER_AGENT'] = REDIS_SSI_USER_AGENT;
  $_SERVER['REQUEST_METHOD'] = 'GET';
  while ((!$timeout || time() < REQUEST_TIME + $timeout) && ($item = $queue->claimItem())) {
    $function = 'redis_ssi_regenerate_' . $item->object_type;
    if (function_exists($function)) {
      $result = $function($item->object_id, $item);
    }
    if ($result !== FALSE) {
      $queue->deleteItem($item);
    }
  }
  // Remove the password, now slaves will reconnect.
//  redis_ssi_client()->config('set', 'requirepass', '');
}

function _drush_redis_ssi_assert_coherency() {
  static $done = FALSE;
  if (!$done) {
    $redis = redis_ssi_client();
    $slaves = array();
    $list = $redis->client('list');
    if ($list) {
      foreach ($list as $client) {
        // This works with any Redis 2.4 although documentation is nil.
        // getClientInfoString in src/networking.src is your guide.
        // https://github.com/antirez/redis/blob/2.4.0/src/networking.c#L902
        if (strpos($client['flags'], 'S') !== FALSE) {
          $slaves[] = $client['addr'];
        }
      }
    }
    if ($slaves) {
      // Password the master and kill the clients to suspend the replication.
      // The slaves will retry connecting until we remove the password.
      $redis->config('set', 'requirepass', 'redis_ssi');
      foreach ($slaves as $slave) {
        $redis->client('KILL', $slave);
      }
    }
    $done = TRUE;
  }
}
