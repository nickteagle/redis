<?php

class RedisSSIQueue {

  public function createItem($object_type, $object_id, $weight = 0) {
    $query = db_insert('redis_ssi_queue')
      ->fields(array(
        'object_id' => (string) $object_id,
        'object_type' => $object_type,
        // We cannot rely on REQUEST_TIME because many items might be created
        // by a single request which takes longer than 1 second.
        'created' => time(),
      ));
    try {
      $query->execute();
      return TRUE;
    }
    catch (PDOException $e) {
      // Ignore unique key violation errors ignnore them and stop content being
      // regen twice. 23505 is the standard SQLSTATE,  which MySQL does not
      // obey, so we also check for the MySQL specific error code 1062.
      // We also test for the standard 23000 duplication error.
      if (!in_array($e->getCode(), array(23000, 23505)) && $e->errorInfo[1] != 1062) {
        throw new PDOException($e);
        watchdog_exception('redis_queue', $e);
        if (itv_misc_is_drush()) {
          return drush_set_error('DRUSH_REDIS_QUEUE_PDO_EXCEPTION', dt('Error: @error', array('@error' => $e->getMessage())));
        }
        else {
          return FALSE;
        }
      }
    }
  }

  public function claimItem($lease_time = 30) {
    return $this->claimItemWithType($lease_time);
  }
  public function claimPageItem($lease_time = 30) {
    return $this->claimItemWithType($lease_time, 'page');
  }
  protected function claimItemWithType($lease_time, $object_type = NULL) {
    $select = db_select('redis_ssi_queue', 'rsq')
      ->fields('rsq', array('object_type', 'object_id', 'item_id', 'retry'))
      ->condition('expire', 0)
      // object type must come first to avoid regenerating the same pages over
      // and over again.
      ->orderBy('retry')
      ->orderBy('item_id')
      ->range(0, 1);
    if (isset($object_type)) {
      $select->condition('object_type', $object_type);
    }
    while (TRUE) {
      $item = $select->execute()->fetchObject();
      if ($item) {
        $update = db_update('redis_ssi_queue')
          ->fields(array(
            'expire' => time() + $lease_time,
          ))
          ->condition('item_id', $item->item_id)
          ->condition('expire', 0);
        if ($update->execute()) {
          return $item;
        }
      }
      else {
        return FALSE;
      }
    }
  }

  public function releaseItem($item) {
    // Items should only be retried a certain number of times to stop the queue filling up with bad URLs.
    $retry_severity = NULL;
    if ($item->retry >= variable_get('redis_queue_retry_warning', 1)) {
      $retry_severity = WATCHDOG_WARNING;
    }
    if ($item->retry >= variable_get('redis_queue_retry_error', 3)) {
      $retry_severity = WATCHDOG_ERROR;
    }
    if ($item->retry >= variable_get('redis_queue_retry', 5)) {
      $retry_severity = WATCHDOG_CRITICAL;
      $this->deleteItem($item);
    }
    if (!empty($retry_severity)) {
      watchdog(
        'PCQ Retry',
        '@count retries for @type url @url',
        array('@count' => $item->retry, '@url' => $item->object_id, '@type' => $item->object_type),
        $retry_severity);
    }
    if ($retry_severity != WATCHDOG_CRITICAL) {
      $update = db_update('redis_ssi_queue')
        ->fields(array(
          'expire' => 0,
          'retry' => $item->retry + 1,
        ))
        ->condition('item_id', $item->item_id);
      return $update->execute();
    }
  }

  public function deleteItem($item) {
    db_delete('redis_ssi_queue')
      ->condition('item_id', $item->item_id)
      ->execute();
  }

  public function numberOfItems() {
    return db_query('SELECT COUNT(item_id) FROM {redis_ssi_queue}')->fetchField();
  }
}
