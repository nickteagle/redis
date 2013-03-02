<?php

/**
 * Replace parts of the page in JavaScript for logged in users.
 *
 * This adds an array of variables to Drupal.settings which are being
 * stored under Drupal.settings.redis
 *
 * @param $account
 *   The user account.
 * @return
 *   Array of values to added to Drupal.settings
 */
function hook_redis_ssi_user_settings($account) {
}
