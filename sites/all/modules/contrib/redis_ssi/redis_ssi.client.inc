<?php

class RedisSSIClient extends Predis\Client {
  function __call($method, $arguments) {
    try {
      return parent::__call($method, $arguments);
    }
    catch (Predis\ServerException $e) {
      // Let's try authenticating and then re-run the command.
      parent::__call('auth', array('redis_ssi'));
      return parent::__call($method, $arguments);
    }
  }
}

class RedisSSIDummyClient {
  // Without __call we would get fatals about nonexistent methods.
  function __call($method, $arguments) {
    // This is a dummy client so do nothing.
  }
}
