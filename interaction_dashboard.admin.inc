<?php
/**
 * @file
 *   Admin related functionalities
 * 
 */
/**
 * 
 */
function interaction_dashboard_status($service) {
  $fn = 'interaction_dashboard_'.str_replace('-', '_', $service);
  $default = array('status' => 2, 'message' => 'Your service not configured proper in service manager!!!');
  $result = array('status' => 2, 'message' => 'Your service not added to service manager yet...');
  if (function_exists($fn)) {
    try {
      $result = $fn();
    }
    catch (Exception $e) {
      $result = array('status' => 2, 'message' => t('Exception:%message', array('%message' => $e->getMessage())));
    }
  }
  if (!is_array($result)) {
    $result = $default;
  }else {
    $result += $default;
  }

  // return drupal_json_output($fn());
  return drupal_json_output($result);
}

/**
 * admin page callback.
 */
function interaction_dashboard_admin_form($form, $form_state) {
  $services = interaction_dashboard_get_all_services();
  foreach ($services as $name => $service) {
    $package = $service->export_module;
    // Set up package elements
    if (!isset($form[$package])) {
      $form[$package] = array(
        '#tree' => FALSE,
        '#title' => $package,
        '#theme' => 'interaction_dashboard_form_package',
        '#type' => 'fieldset',
        '#group' => 'packages',
      );
      $form[$package]['links'] =
      $form[$package]['version'] =
      $form[$package]['weight'] =
      $form[$package]['status'] =
      $form[$package]['action'] = array('#tree' => TRUE);
    }
    $form[$package]['status'][$name] = array(
      '#type' => 'item',
      '#title' => $service->machine_name,
      '#description' => truncate_utf8($service->description, 40, FALSE, TRUE),
    );
    $href = '';
    $state = theme('interaction_dashboard_storage_link', array('storage' => INTERACTION_DASHBOARD_CHECKING, 'path' => $href));
    $state .= l(t('Check'), "admin/structure/test-service/{$name}/status", array('attributes' => array('class' => array('admin-check'))));
    $state .= theme('interaction_dashboard_storage_link', array('storage' => INTERACTION_DASHBOARD_RUNNING, 'path' => $href));
    $state .= theme('interaction_dashboard_storage_link', array('storage' => INTERACTION_DASHBOARD_FAILING, 'path' =>  $href));
    $state .= theme('interaction_dashboard_storage_link', array('storage' => INTERACTION_DASHBOARD_RELOAD, 'path' => '#'));
    
    $form[$package]['state'][$name] = array(
      '#markup' => $state,
    );
  }
  ksort($form);
  $form = array('packages' => array('#type' => 'vertical_tabs')) + $form;
  return $form;
}

function interaction_dashboard_get_all_services() {
  return messagequeue_service_load_all();
}