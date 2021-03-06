<?php

/**
 * @file
 *
 * Plugin to provide an argument handler for all entity ids
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'title' => t("Entity: ID"),
  'description' => t('Creates an entity context from an entity ID argument.'),
  'context' => 'ctools_argument_entity_id_context',
  'get child' => 'ctools_argument_entity_id_get_child',
  'get children' => 'ctools_argument_entity_id_get_children',
);

function ctools_argument_entity_id_get_child($plugin, $parent, $child) {
  $plugins = ctools_argument_entity_id_get_children($plugin, $parent);
  return $plugins[$parent . ':' . $child];
}

function ctools_argument_entity_id_get_children($original_plugin, $parent) {
  $entities = entity_get_info();
  $plugins = array();
  foreach ($entities as $entity_type => $entity) {
    $plugin = $original_plugin;
    $plugin['title'] = t('@entity: ID', array('@entity' => $entity['label']));
    $plugin['keyword'] = $entity_type;
    $plugin['description'] = t('Creates @entity context from an ID argument.', array('@entity' => $entity_type));
    $plugin['name'] = $parent . ':' . $entity_type;
    $plugin_id = $parent . ':' . $entity_type;
    drupal_alter('ctools_entity_context', $plugin, $entity, $plugin_id);
    $plugins[$plugin_id] = $plugin;
  }
  drupal_alter('ctools_entity_contexts', $plugins);
  return $plugins;
}

/**
 * Discover if this argument gives us the entity we crave.
 */
function ctools_argument_entity_id_context($arg = NULL, $conf = NULL, $empty = FALSE) {
  $entity_type = explode(':', $conf['name']);
  $entity_type = $entity_type[1];
  // If unset it wants a generic, unfilled context.
  if ($empty) {
    return ctools_context_create_empty('entity:' . $entity_type);
  }

  // We can accept either an entity object or a pure id.
  if (is_object($arg)) {
    return ctools_context_create('entity:' . $entity_type, $arg);
  }

  if (!is_numeric($arg)) {
    return FALSE;
  }

  $entity = entity_load($entity_type, array($arg));
  if (!$entity) {
    return FALSE;
  }

  return ctools_context_create('entity:' . $entity_type, $entity[$arg]);
}

