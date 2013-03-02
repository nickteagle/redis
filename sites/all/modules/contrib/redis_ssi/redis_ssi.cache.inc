<?php

class RedisSSICache implements DrupalCacheInterface {

  function __construct($bin) {
    $this->nginx_path = variable_get('redis_ssi_nginx_path', '/redis');
  }

  /**
   * Note that $data is passed-by-reference, *by the calling stack*. 
   */
  function set($cid, $data, $expire = CACHE_PERMANENT) {
    list($hash, $ssi_key) = explode(':', $cid, 2);
    $markup = $data['#markup'];
    $redis_key = $ssi_key;
    if ($hash == 'node') {
      if ($ssi_key[0] == '!') {
        // We are in the middle of a multiple view. Remove the markup,
        // the SSI command will be a multiple GET at the last node.
        $data['#markup'] = '';
        // Remove !:
        $redis_key = substr($ssi_key, 2);
      }
      else {
        // We only need to save the last node to redis but we need to
        // use the full key in the SSI command -- this is why we have a
        // separate $ssi_key and $redis_key.
        $ssi_keys = explode('/', $ssi_key);
        $redis_key = array_pop($ssi_keys);
      }
    }
//error_log('set='.print_r($markup,1));
    redis_ssi_client()->hset($hash, str_replace(array('&', '?'), ':', $redis_key), $markup);
    if ($data['#markup']) {
      $data['#markup'] = "<!--# include virtual=\"$this->nginx_path/$hash/$ssi_key\" -->";
    }
  }


  function get($cid) {
/*    list($hash, $ssi_key) = explode(':', $cid, 2);
    // For nodes, if we are not generating fragments, return the same markup
    // as set returns in $data['#markup'].
    if ($hash == 'node' && arg(2) != 'fragment') {
      // If we are here, then we are called from drupal_render_cache_get()
      // because nothing else uses this cache backend. That function only
      // uses $cache->data['#markup'] so set that.
      // @see redis_ssi_node_view_alter() about the ! marking the middle of a
      // multiple view.
      $cache->data['#markup'] = $ssi_key[0] == '!' ? '' : "<!--# include virtual=\"$this->nginx_path/$hash/$ssi_key\" -->";
      return $cache;
    }*/
    return FALSE;
  }

  function getMultiple(&$cids) {
    return array();
  }

  function clear($cid = NULL, $wildcard = FALSE) {
  }

  function isEmpty() {
    return TRUE;
  }
}
