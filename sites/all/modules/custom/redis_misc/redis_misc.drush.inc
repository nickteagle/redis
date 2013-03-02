<?php
/**
 * @file
 * Drush integration of player_cache.module.
 */

/**
 * Implements hook_drush_help().
 */
function redis_misc_drush_help($section) {
  switch ($section) {
    case 'drush:redis-misc-queue-process':
      return dt('Process the Jobs on the Redis queue.');
  }
}

/**
 * Implements hook_drush_command().
 */
function redis_misc_drush_command() {
  $items['redis-misc-queue-process'] = array(
    'description' => dt('Process the Redis queue.'),
    'aliases' => array('pcq'),
    'options' => array(
      'timeout' => dt('How long this function should run.'),
    ),
  );
  return $items;
}

/**
 * Implements drush_hook_COMMAND().
 */
function drush_redis_misc_queue_process() {
  // Show output.
  $timeout = drush_get_option('timeout', 55);
  $queue = redis_ssi_queue_get();
  $queue_size = $queue->numberOfItems();
  $_SERVER['HTTP_USER_AGENT'] = REDIS_SSI_USER_AGENT;
  $_SERVER['REQUEST_METHOD'] = 'GET';
  $paths_processed = array();
    while ((time() < REQUEST_TIME + $timeout) && ($queue_size > 0)) {
      $paths = redis_misc_regenerate_page();
      $queue_size = $queue->numberOfItems();
    }
}


