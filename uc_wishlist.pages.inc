<?php
// $Id: uc_wishlist.pages.inc,v 1.1 2009/03/27 16:08:33 rszrama Exp $

/**
 * @file
 * Page callback and functions for wish lists.
 */


/**
 * Display a wish list for viewing or altering.
 *
 * @param $wid
 *   The ID of the wish list to display.
 * @return
 *   HTML output for the wish list display and form.
 */
function uc_wishlist_display($wid = NULL) {
  global $user;
  $output = '';
  drupal_add_js('sites/all/themes/mightynest/limitchars.js');

  // $own defines whether the wish list is owned by the current user or not.
  // This affects how many parts of the page are rendered.
  $own = FALSE;

  // Default to the current user's wish list if no wishlist ID is specified.
  if (empty($wid)) {
    $wid = uc_wishlist_get_wid();
    $own = TRUE;
  }
  elseif ($wid == uc_wishlist_get_wid()) {
    $own = TRUE;
  }

  // Attempt to load the wish list.
  $wishlist = uc_wishlist_load($wid);

  // Handle a non-existent wish list.
  if (!$wishlist) {
    // If the requested list was for the current user...
    if ($own) {
      // Display a message letting them know their list is empty.
      drupal_set_title(t('Wish list'));
      

      return '<p><a href="/shop" class="start">Sxtart building your Wish List &#187;</a></p>';
    }
    else {
      // Otherwise send them to the search form.
      drupal_set_message(t('The wish list you requested could not be found.  Perhaps you can try looking for it through the wish list search form below.'));
      drupal_goto('wishlist/search');
    }
  }
  
  if ($wishlist->address->firstname) {
    $page_title = $wishlist->address->firstname . "'s ";
  }
  
  $page_title .= 'Wish List';

  drupal_set_title(check_plain($page_title));

  // Add the settings form if the user is viewing his own wish list.
  if ($own) {
    if (!$user->uid) {
      drupal_set_message(t('You must <a href="!login_url">login</a> or <a href="!register_url">register</a> to save your wish list.', array('!login_url' => url('user/login'), '!register_url' => url('user/register'))));
    }

    $collapsed = TRUE;

    if (empty($wishlist->address->firstname) || empty($wishlist->address->lastname) || empty($wishlist->address->addr1) || empty($wishlist->address->city) || empty($wishlist->address->postcode)) {
      $collapsed = FALSE;
    }

    $output .= drupal_get_form('uc_wishlist_settings_form', $wishlist, $collapsed);
  }

  $items = uc_wishlist_get_contents($wid);
  
  if ($wishlist->address->description) {
    $wishlist_info = '<div class="wishlist-description">' . filter_xss($wishlist->address->description) . '<span class="name">-' . filter_xss($wishlist->address->firstname) . ' ' . $wishlist->address->lastname ;
    if ($wishlist->address->company) {
      $wishlist_info .= ' and ' . filter_xss($wishlist->address->company);
    }    
    $wishlist_info .= '</span></div>';
  }
  
  if ($user->uid && !is_numeric(arg(1))) {
    $addthis_url = 'addthis:url="http://mightynest.com/wishlist/' . $wid . '"';
  }
  
  $wishlist_info .= '<div class="wlist-share"><div class="addthis_toolbox"><div class="custom_images"><a ' . $addthis_url . ' class="addthis_button_email"><img src="/sites/all/themes/mightynest/images/share-email.gif" width="25" height="25" alt="Email" /></a><a ' . $addthis_url . ' class="addthis_button_facebook"><img src="/sites/all/themes/mightynest/images/share-facebook.gif" width="26" height="25" alt="Facebook" /></a><a ' . $addthis_url . ' class="addthis_button_twitter"><img src="/sites/all/themes/mightynest/images/share-twitter.gif" width="26" height="25" alt="Twitter" /></a></div></div><script type="text/javascript" src="http://s7.addthis.com/js/250/addthis_widget.js#username=xa-4b4e48877468911f"></script><script type="text/javascript">var addthis_share = {"url":\'' . $addthis_url . '\',"title":"' . check_plain($wishlist->address->firstname) . '\'s Wish List", "email_template": \'wishlist_share\'};</script><!-- AddThis Button END -->Share this Wish List!</div>';
  
  $wishlist_info .= '<span class="names">' . filter_xss($wishlist->address->firstname) . ' ' . $wishlist->address->lastname . '</span>';
  if ($wishlist->address->company) {
    $wishlist_info .= '<br /><span class="names">' . filter_xss($wishlist->address->company) . '</span>';
  }
  if ($wishlist->address->city) {
    $wishlist_info .= '<br />' . filter_xss($wishlist->address->city) . ', ' . uc_zone_get_by_id($wishlist->address->zone);
  }
  $event_info = array();
  if ($wishlist->address->event_type) {
    $event_info[] = filter_xss($wishlist->address->event_type);
  }
  if ($wishlist->address->firstname && $wishlist->expiration != 0) {
    $event_info[] = date('n/d/Y',$wishlist->expiration+21600);
  }
  if ($wishlist->address->firstname && ($wishlist->expiration || $wishlist->address->event_type)) {
    $wishlist_info .= '<br /><br /><span class="event-date">Event: ' . implode(', ',$event_info) . '</span>';
  }

  if (empty($items)) {
    if ($own) {
      $output .= '<p><a href="/shop" class="start">Start building your Wish List &#187;</a></p>';
    }
    else {
      $output .= '<p><i>No products have been added to this Wish List.</i></p>';
    }
  }
  else {
    $output .= '<div id="uc-wishlist-view-form">' . drupal_get_form('uc_wishlist_view_form', $items, $wid, $own) . '</div>';
  }
  if (!$user->uid) {
    $wishlist_info .= '<br /><br />Is this your Wish List? <a href="/user/login?destination=wishlist">Click here.</a>';
  }

  return '<span class="wishlist-info">' . $wishlist_info . '</span>' . $output;
}

// Build the user wish list settings form.
function uc_wishlist_settings_form($form_state, $wishlist, $collapsed = FALSE) {
  global $user;

  drupal_add_js(drupal_get_path('module', 'uc_wishlist') .'/uc_wishlist.js');


  // Attempt to load the wish list.
  $wid = uc_wishlist_get_wid();
  $wishlist = uc_wishlist_load($wid);
  // If there's no first name, this is the first time the user has filled out the form. Do not pre-fill expiration.
  if (!$wishlist->address->firstname) {
    $expiration = '';
  }
  elseif ($wishlist->expiration) {
    $expiration = date_convert($wishlist->expiration,DATE_UNIX,DATE_DATETIME);
  }

  $form = array();
  if (is_numeric($_GET['nid'])) {
    $form['product_nid'] = array(
      '#type' => 'hidden',
      '#value' => $_GET['nid'],
    );
    if (is_numeric($_GET['qty'])) {
      $form['product_qty'] = array(
        '#type' => 'hidden',
        '#value' => $_GET['qty'],
      );
    }
    if ($_GET['data']) {
      $form['product_data'] = array(
        '#type' => 'hidden',
        '#value' => filter_xss($_GET['data'])
      );
    }
  }
  
  if (arg(1) == 'create') {
    $submit_value = "Create my Wish List";
    $form['create_new'] = array(
      '#type' => 'hidden',
      '#value' => 'create_new',
    );
    $title = check_plain($user->name) . '\'s Wish List';
  }
  else {
    $submit_value = "Save";
    $title = $wishlist->title;
  }

  $form['wishlist'] = array(
    '#type' => 'fieldset',
    '#title' => t('Wish list settings'),
    '#collapsible' => TRUE,
    '#collapsed' => $collapsed,
  );
  $form['wishlist']['wid'] = array(
    '#type' => 'hidden',
    '#value' => $wishlist->wid,
  );
  $form['wishlist']['title'] = array(
    '#type' => 'hidden',
    '#title' => t('Edit your wish list information'),
    '#default_value' => $title,
    '#required' => TRUE,
  );
  $form['wishlist']['expiration'] = array(
    '#type' => 'date',
    '#title' => t('Event or expiration date'),
    '#default_value' => $expiration,
    '#description' => t('If this wish list is associated with an event or will no longer be relevant on a specific date, enter it here.'),
  );
  
  if ($user->uid) {
    $addresses = uc_select_address($user->uid, 'delivery', 'apply_address(\'delivery\', this.value);', t('Saved addresses'), TRUE);
    if (!empty($addresses)) {
      $form['wishlist']['delivery_address_select'] = $addresses;
      unset($form['wishlist']['delivery_address_select']['#suffix']);
    }
  }

  if (uc_address_field_enabled('first_name')) {
    $form['wishlist']['delivery_first_name'] = uc_textfield(uc_get_field_name('first_name'), $wishlist->address->firstname, uc_address_field_required('first_name'));
  }
  if (uc_address_field_enabled('last_name')) {
    $form['wishlist']['delivery_last_name'] = uc_textfield(uc_get_field_name('last_name'), $wishlist->address->lastname, uc_address_field_required('last_name'));
  }
  if (uc_address_field_enabled('company')) {
    $form['wishlist']['delivery_company'] = uc_textfield('Additional name', $wishlist->address->company, uc_address_field_required('company'), NULL, 64);
  }
  if (uc_address_field_enabled('street1')) {
    $form['wishlist']['delivery_street1'] = uc_textfield(uc_get_field_name('street1'), $wishlist->address->addr1, uc_address_field_required('street1'), NULL, 64);
  }
  if (uc_address_field_enabled('street2')) {
    $form['wishlist']['delivery_street2'] = uc_textfield(uc_get_field_name('street2'), $wishlist->address->addr2, uc_address_field_required('street2'), NULL, 64);
  }
  if (uc_address_field_enabled('city')) {
    $form['wishlist']['delivery_city'] = uc_textfield(uc_get_field_name('city'), $wishlist->address->city, uc_address_field_required('city'));
  }
  if (uc_address_field_enabled('country')) {
    $form['wishlist']['delivery_country'] = uc_country_select(uc_get_field_name('country'), $wishlist->address->country, NULL, 'name', uc_address_field_required('country'));
  }
  if (uc_address_field_enabled('zone')) {
    if (isset($_POST['delivery_country'])) {
      $country_id = intval($_POST['delivery_country']);
    }
    else {
      $country_id = $wishlist->address->country;
    }
    $form['wishlist']['delivery_zone'] = uc_zone_select(uc_get_field_name('zone'), $wishlist->address->zone, NULL, $country_id, 'name', uc_address_field_required('zone'));
    if (isset($_POST['panes']) && count($form['wishlist']['address']['delivery_zone']['#options']) == 1) {
      $form['wishlist']['address']['delivery_zone']['#required'] = FALSE;
    }
  }
  if (uc_address_field_enabled('postal_code')) {
    $form['wishlist']['delivery_postal_code'] = uc_textfield(uc_get_field_name('postal_code'), $wishlist->address->postcode, uc_address_field_required('postal_code'), NULL, 10, 10);
  }
  if (uc_address_field_enabled('phone')) {
    $form['wishlist']['delivery_phone'] = uc_textfield(uc_get_field_name('phone'), $wishlist->address->phone, uc_address_field_required('phone'), NULL, 32, 16);
  }
  unset($form['wishlist']['expiration']);
  $form['wishlist']['event'] = array(
    '#type' => 'fieldset',
    '#collabsible' => FALSE,
    '#title' => 'Is this for an event?',
  );    
  $form['wishlist']['event']['event_type'] = array(
    '#type'   => 'textfield',
    '#title'  => t('Event type'),
    '#default_value' => $wishlist->address->event_type,
    '#required'   => FALSE,
    '#size' => 32,
    '#description' => t('Example: birthday, new baby, Christmas, etc.'),
    '#weight' => 7,
  );
  $form['wishlist']['event']['expiration'] = array(
    '#type'   => 'date_select',
    '#title'  => t('Event date'),
    '#date_format' => 'm-d-Y',
    '#date_year_range' => '0:+20',
    '#default_value' => $expiration,
    '#required'   => FALSE,
    '#weight' => 8,
  );
  $form['wishlist']['event']['description'] = array(
    '#type' => 'textarea',
    '#title' => 'Personal message',
    '#default_value' => $wishlist->address->description,
    '#weight' => 9,
    '#resizable' => FALSE,
    '#cols' => 35,
    '#rows' => 3,
    '#suffix' => '<div id="wltext-limit">You have 250 characters left.</div>',
  );

  $form['wishlist']['save'] = array(
    '#type' => 'submit',
    '#value' => $submit_value,
    '#weight' => 10,
  );

  return $form;
}

function uc_wishlist_settings_form_validate($form, &$form_state) {
  global $user;
  
  if ($form_state['values']['create_new'] != 'create_new') {
    $wishlist = uc_wishlist_load($wid);
    if (!$wishlist) {
      drupal_set_message(t('Could not find the specified Wish List.'), 'error');
      return FALSE;
    }
    if ($wishlist->uid != $user->uid && !user_perm('administer store')) {
      drupal_set_message(t('You do not have permission to edit this Wish List.'), 'error');
      return FALSE;
    }
  }
}

function uc_wishlist_settings_form_submit($form, &$form_state) {
  
  if ($form_state['values']['create_new'] == 'create_new') {
    $wid = uc_wishlist_create_wishlist();
    if (!$wid) {
      drupal_set_message(t('Could not create Wish List. Adding item failed.'), 'error');
      return FALSE;
    }
  }
  $expiration = date_convert($form_state['values']['expiration'],DATE_DATETIME,DATE_UNIX);

  $address = array(
    'firstname' => $form_state['values']['delivery_first_name'],
    'lastname' => $form_state['values']['delivery_last_name'],
    'company' => $form_state['values']['delivery_company'],
    'addr1' => $form_state['values']['delivery_street1'],
    'addr2' => $form_state['values']['delivery_street2'],
    'city' => $form_state['values']['delivery_city'],
    'country' => $form_state['values']['delivery_country'],
    'zone' => $form_state['values']['delivery_zone'],
    'postcode' => $form_state['values']['delivery_postal_code'],
    'phone' => $form_state['values']['delivery_phone'],
    'description' => $form_state['values']['description'],
    'event_type' => $form_state['values']['event_type']
  );
  
  $wishlist_name = $form_state['values']['delivery_first_name'] . ' ' . $form_state['values']['delivery_last_name'];
  if ($form_state['values']['co_registrant'] != '') {
    $wishlist_name .= ' and ' . $form_state['values']['co_registrant'];
  }
  $new_title = $wishlist_name . '\'s Wish List';
  
  if ($form_state['values']['create_new'] == 'create_new') {
    $wid = uc_wishlist_get_wid();
  }
  else {
    $wid = $form_state['values']['wid'];
  }
  
  if ($form_state['values']['product_nid']) {
    parse_str($form_state['values']['product_data'],$data);
    uc_wishlist_add_item($form_state['values']['product_nid'], $form_state['values']['product_qty'], $data, $wid);
  }

  uc_wishlist_update_wishlist($wid, $new_title, $expiration, (object) $address);

  if ($form_state['values']['create_new'] == 'create_new') {
    drupal_set_message(t('Your Wish List has been created. <a href="/shop">Start shopping now >></a>'));
  }
  else {
    drupal_set_message(t('Your Wish List has been updated.'));
  }
  drupal_goto('wishlist');
}

/**
 * Display a page allowing the customer to view his/her wish list.
 */
function uc_wishlist_view_form($form_state, $items, $wid, $own) {
  $form = array();
    
      $sets = array();
      foreach ($items as $index => $row) {
        $grouping = '';
        //if (isset($this->view->field[$grouping_field])) {
          $grouping = $row->tid;
          //if ($this->view->field[$grouping_field]->options['label']) {
            $grouping = $grouping;
          //}
        //}
        $sets[$grouping][$index] = $row;
      }

  
  $form['sets'] = array(
    //'#tree' => TRUE,
  );

  foreach ($sets as $index => $wlst_items) {
    // Load each wish list product and add it to the form array.
    // $index = our term id
    $form['sets'][$index]['#tree'] = TRUE;
    $form['sets'][$index]['items'] = array(
      '#tree' => TRUE,
    );
    foreach ($wlst_items as $item) {
      $node = node_load($item->nid);
      $element = array();
  
      $element['nid'] = array('#type' => 'value', '#value' => $node->nid);
      $element['wpid'] = array('#type' => 'value', '#value' => $item->wpid);
      $element['module'] = array('#type' => 'value', '#value' => 'uc_product');
  
      if ($own) {
        $element['remove'] = array('#type' => 'checkbox');
      }
  
      $item->haveqty = 0;
      if (is_array($item->purchase)) {
        $item->haveqty = count($item->purchase);
      }
      
      if ($node->field_manufacturer[0]['nid']) {
        $brand = node_load($node->field_manufacturer[0]['nid']);
        $brand_title = ' by ' . $brand->title;
      }
  
      $element['title'] = array(
        '#value' => '<span class="title"><b>' . filter_xss($node->title) . '</b>' . filter_xss($brand_title) . '</span>',
      );
  
      $description = '';
			
			// Add any attribute descriptions:
			$description .= uc_product_get_description($item);
			
      $description .=	' <div><a href="#TB_inline?height=300&width=400&inlineId=full-description-' . $node->nid . '" title="' .  $node->title . '" class="thickbox">Product description &#187;</a></div><div id="full-description-' . $node->nid . '" style="display:none">' . check_markup($node->body,2,FALSE) . '</div>';
      
      if ($description) {
        $element['description'] = array('#value' => $description);
      }
  
      $element['#total'] = $item->price;
  
      $element['data'] = array('#type' => 'hidden', '#value' => serialize($item->data));
  
      $element['wantqty'] = array(
        '#type' => 'textfield',
        //'#value' => $item->qty,   // Necessary with Disabled
        '#default_value' => $item->qty,
        '#size' => 5,
        '#maxlength' => 6,
        '#disabled' => $own ? FALSE : TRUE
      );
      if (!$own) {
        $element['wantqty']['#value'] = $item->qty;
      }
      $element['haveqty'] = array(
        '#type' => 'textfield',
        '#value' => $item->haveqty,   // Necessary with Disabled
        '#default_value' => $item->haveqty,
        '#size' => 5,
        '#maxlength' => 6,
        '#disabled' => TRUE
      );
      $element['qty'] = array(
        '#type' => 'textfield',
        '#size' => 5,
        '#maxlength' => 6,
        '#default_value' => '',
      );
      if (!variable_get('uc_wishlist_show_combined_addcart_button', null)) {
        $element['addcart'] = array(
          '#type' => 'submit',
          '#name' => 'addcart-'. $item->wpid,
          '#value' => t('Add to cart'),
        );
      } 
  
      $form['sets'][$index]['items'][] = $element;
    }
  } 
  
  $form['buttons_top'] = array(
    '#type' => 'fieldset',
    '#attributes' => array('class' => 'buttons'),
    '#weight' => -99,
  );
  
  if ($own) {
    $form['buttons_top']['continue'] = array(
      '#type' => 'markup',
      '#value' => '<a class="wlist-continue" href="/shop">Add more products to your Wish List</a>',
    );
  }
  
  if (variable_get('uc_wishlist_show_combined_addcart_button', null)) {
    $form['buttons_top']['addcart_top'] = array(
      '#type' => 'submit',
      '#value' => t('Add Items to Cart'),
    );
  }
  
  $form['buttons'] = array(
    '#type' => 'fieldset',
    '#attributes' => array('class' => 'buttons'),
  );
  
  if ($own) {
    $form['buttons']['continue'] = array(
      '#type' => 'markup',
      '#value' => '<a class="wlist-continue" href="/shop">Add more products to your Wish List</a>',
    );
  }
  
  if (variable_get('uc_wishlist_show_combined_addcart_button', null)) {
    $form['buttons']['addcart'] = array(
      '#type' => 'submit',
      '#value' => t('Add Items to Cart'),
    );
  }

  if ($own) {
    $form['own'] = array(
      '#type' => 'value',
      '#value' => TRUE,
    );
    $form['buttons']['update'] = array(
      '#type' => 'submit',
      '#value' => t('Update wish list'),
    );
    $form['buttons_top']['update_top'] = array(
      '#type' => 'submit',
      '#value' => t('Update wish list'),
    );
  }

  $form['wid'] = array(
    '#type' => 'hidden',
    '#value' => $wid,
  );
  
  return $form;
}

function theme_uc_wishlist_view_form($form) {
  //print_r($form);
  
  $show_combined_addcart_button = FALSE;
  if (variable_get('uc_wishlist_show_combined_addcart_button', null)) {
    $show_combined_addcart_button = TRUE;
  }
  
  $form_top = drupal_render($form['buttons_top']);
  
  $own = isset($form['own']) ? TRUE : FALSE;

  if ($show_combined_addcart_button) {
    $header = array(array('data' => 'Products <span class="wlist-total">(Total number of products in Wish List: ' . count($form['#parameters'][2]) . ')</span>','class' => 'products'), '', array('data' => t('Price'), 'align' => 'right', 'class' => 'total'), array('data' => t('Wants'), 'class' => 'wanted'), array('data' => t('Has'), 'class' => 'have'), array('data' => t('Purchase'), 'class' => 'qty'));
  }
  else {
    $header = array(array('data' => t('Products'),'class' => 'products'), array('data' => t('Price'), 'align' => 'right', 'class' => 'total'), array('data' => t('Wants'), 'class' => 'wanted'), array('data' => t('Has'), 'class' => 'have'), array('data' => t('Purchase'), 'class' => 'qty'), array('data' => t(''), 'class' => 'purchase'));
  }

  if ($own) {
    array_unshift($header, array('data' => t('Remove'), 'class' => 'remove'));
    $attributes = 'wishlist-own';
  }
  else {
    $attributes = 'wishlist-other';
  }
  // we want to change this to foreach(sets), foreach $form['set']['items'], blah
  foreach ($form['sets'] as $index => $set) {
    // hack, because i can't get element_children to work like i'd expect
    if (is_numeric($index)) {
      $rows = array();
      foreach (element_children($set['items']) as $i) {
        $nid = $set['items'][$i]['nid']['#value'];
        $product = node_load($nid);
        $thumb = theme('imagecache','product_list',$product->field_image_cache[0]['filepath'], $product->title, '');
        $preview = imagecache_create_path('product_preview',$product->field_image_cache[0]['filepath']);
        
        $desc = drupal_render($form['sets'][$index]['items'][$i]['title']);
        $desc .= drupal_render($form['sets'][$index]['items'][$i]['description']);
        $row = array(
          //uc_product_get_picture($nid, 'cart'),
          array('data' => l($thumb,$preview,array('html' => TRUE,'attributes' => array('class' => 'thickbox'))), 'class' => 'image'),
          array('data' => $desc, 'class' => 'description'),
          array('data' => '<span class="price">' . uc_currency_format($form['sets'][$index]['items'][$i]['#total']) . '</span>', 'nowrap' => 'nowrap', 'class' => 'total'),
          $set['items'][$i]['wantqty'] ? array('data' => drupal_render($form['sets'][$index]['items'][$i]['wantqty']), 'class' => 'wanted') : '',
          $set['items'][$i]['haveqty'] ? array('data' => drupal_render($form['sets'][$index]['items'][$i]['haveqty']), 'class' => 'have') : '',
          $set['items'][$i]['qty'] ? array('data' => drupal_render($form['sets'][$index]['items'][$i]['qty']), 'class' => 'qty') : '',
        );
        if (!$show_combined_addcart_button) {
          array_push(&$row, $set['items'][$i]['addcart'] ? drupal_render($form['sets'][$index]['items'][$i]['addcart']) : '');
        }
    
        if (isset($set['items'][$i]['remove'])) {
          array_unshift($row, array('data' => drupal_render($form['sets'][$index]['items'][$i]['remove']), 'class' => 'remove'));
        }
    
        $rows[] = array('data' => $row, 'valign' => 'top');
      }
      $term = taxonomy_get_term($index);
      // we want, basically, foreach $sets, theme(table) with a caption of the taxo name
      $output .= '<div class="wishlist-form-products">'. theme('table', NULL, $rows,'',$term->name) .'</div>';
    }
    
  }

  return '<div class="' . $attributes . '">' . $form_top . theme('table',$header,NULL) . $output . drupal_render($form) . '</div>';
}

function uc_wishlist_view_form_validate($form, &$form_state) {
  // Find wish list item associated with 'add to cart' submitted.
  foreach ($form_state['values'] as $key => $val) {
    if (strpos($key, 'addcart-') === 0) {
      $wpid = intval(substr($key, 8));
    }
  }
  // If adding item to cart, check quantities.
  if ($wpid) {
    foreach ($form_state['values'] as $key => $item) {
      /* In case the customer clicks the 'add to cart' for
       * this item after setting the qty to 0, they should
       * not get an error, since they probably mean to purchase
       * this item. So set qty=1 if qty==0
       */
      if (is_array($item)) {
        foreach ($item['items'] as $pkey => $product) {
          if ($item['items'][$pkey]['wpid'] == $wpid) {
            if ($item['items'][$pkey]['qty'] === "0" || $item['items'][$pkey]['qty'] == "") {
              $form_state['values'][$key]['items'][$pkey]['qty'] = 1;
            }
            elseif (!is_numeric($item['items'][$pkey]['qty'])) {
              form_set_error($key . '][items][' . $pkey . '][qty', t('You have entered an invalid quantity. '));
            }
          }
        }
      }
    }
  }
}

function uc_wishlist_view_form_submit($form, &$form_state) {
  //print_r($form_state);
  if (isset($form_state['values']['op']) && !empty($form_state['values']['op'])) {
    if ($form_state['values']['op'] == $form_state['values']['update']) {
      //print_r((object) $form_state['values']);
      uc_wishlist_update_item_object((object) $form_state['values']);

      drupal_set_message(t('Your wish list has been updated.'));

      $form_state['redirect'] = 'wishlist/'. $form_state['values']['wid'];
    }
    // Add all items to cart
    if ($form_state['values']['op'] == $form_state['values']['addcart']) {
      uc_wishlist_add_items_to_cart((object) $form_state['values']);
    }
  }
  else {
    // Find wish list item associated with 'add to cart' submitted.
    foreach ($form_state['values'] as $key => $val) {
      if (strpos($key, 'addcart-') === 0) {
        $wpid = intval(substr($key, 8));
      }
    }

    if ($wpid) {
      foreach ($form_state['values'] as $key => $item) {
        /*
         * uc_registry_add_items_to_cart() does not have a way
         * for us to define which specific item we want to purchase.
         * It takes in the whole form. So we have to unset the
         * quantities on those elements that we don't want to purchase!
         */
        if (is_array($item)) {
          foreach ($item['items'] as $pkey => $product) {
            if ($item['items'][$pkey]['wpid'] == $wpid) {
              if ($item['items'][$pkey]['qty'] === "0" || $item['items'][$pkey]['qty'] == "") {
                $form_state['values'][$key]['items'][$pkey]['qty'] = 1;
              }
            }
            else {
              $form_state['values'][$key]['items'][$pkey]['qty'] = 0;
            }
          }
        }
      }

      uc_wishlist_add_items_to_cart((object) $form_state['values']);
    }
  }
}

// Display the wish list search form.
function uc_wishlist_search_form($form_state, $keywords = '') {
  global $user;
  $form = array();
  
  $first_name = check_plain($_GET['first_name']);
  $last_name = check_plain($_GET['last_name']);
  $city = check_plain($_GET['city']);
  $country = check_plain($_GET['country']);
  $zone = check_plain($_GET['zone']);
  
  
  
  if (arg(1) == 'search') {
    $query_args = array();
    $where = array();
    
    if (!empty($first_name)) {
      $where[] = "w.address LIKE '%%%s%%'";
      $query_args[] = $first_name;
    }
    
    if (!empty($last_name)) {
      $where[] = "w.address LIKE '%%%s%%'";
      $query_args[] = $last_name;
    }
    
    if (!empty($city)) {
      $where[] = "w.address LIKE '%%city%%%s%%'";
      $query_args[] = $city;
    }
    
    if (!empty($country)) {
      $where[] = "w.address LIKE '%%country%%%s%%'";
      $query_args[] = $country;
    }
    
    if (!empty($zone)) {
      $where[] = "w.address LIKE '%%zone%%%s%%'";
      $query_args[] = $zone;
    }
    
    $where = implode(' AND ',$where);
  
    if (!empty($first_name) || !empty($last_name) || !empty($city) || !empty($country) || !empty($zone)) {
      // Check for user, wish list title, or address matches.
      $result = db_query("SELECT DISTINCT w.wid, w.title, w.address, w.expiration FROM {uc_wishlists} AS w JOIN {users} AS u ON w.uid = u.uid WHERE " . $where . " ORDER BY w.title", $query_args);
      $search_executed = TRUE;
    }
    else {
      //$result = pager_query("SELECT wid, title FROM {uc_wishlists} ORDER BY title", 25);
    }
    if (arg(1) == 'search') { 
      $form_prefix = '<div class="search-again">Didn\'t find what you were looking for? Search again:</div>';
    }
    
    $rows = array();
    $i=1;
    while ($wishlist = db_fetch_object($result)) {
      if ($wishlist->expiration != 0) {
        $res_expiration = date('n/d/Y',$wishlist->expiration);
      }
      $address = unserialize($wishlist->address);
      $rows[] = array(
        array('data' => $i,'class' => 'number'),
        array('data' => l(filter_xss($address->lastname, array()) . ', ' . filter_xss($address->firstname, array()),'wishlist/' . $wishlist->wid),'class' => 'name'),
        array('data' => filter_xss($address->company),'class' => 'add_name'),
        array('data' => filter_xss($address->city, array()) . ', ' . uc_zone_get_by_id($address->zone),'class' => 'location'),
        array('data' => $res_expiration,'class' => 'date'),
      );
      $i++;
    }
  
    if (!empty($rows)) {
      $result_count = count($rows) . t(' Results:');
      $output = theme('table',array('#','Name','Additional name','Location','Event date'),$rows);
    }
    else {
      $output = t('<div class="result-count">No wish lists found.</div>');
    }
  

    $form['output'] = array(
      '#value' => '<div class="nests"> </div><div><div class="result-count">' . $result_count .'</div>'. $output .'</div>',
    );
  }

  $form['search'] = array(
    '#type' => 'fieldset',
    '#prefix' => $form_prefix,
  );
  $form['search']['first_name'] = array(
    '#type' => 'textfield',
    '#title' => t('First Name'),
    '#default_value' => $first_name,
    '#size' => 20,
  );
  $form['search']['last_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Last Name'),
    '#default_value' => $last_name,
    '#size' => 20,
  );
  $form['search']['city'] = array(
    '#type' => 'textfield',
    '#title' => t('City'),
    '#default_value' => $city,
    '#size' => 20,
  );
  if (uc_address_field_enabled('country')) {
    $form['search']['delivery_country'] = uc_country_select(uc_get_field_name('country'), '', NULL, 'name', FALSE);
  }
  if (uc_address_field_enabled('zone')) {
    if (isset($_POST['delivery_country'])) {
      $country_id = intval($_POST['delivery_country']);
    }
    else {
      $country_id = $wishlist->address->country;
    }
    $form['search']['delivery_zone'] = uc_zone_select(uc_get_field_name('zone'), $_POST['delivery_zone'], NULL, $country_id, 'name', FALSE);
    if (isset($_POST['panes']) && count($form['search']['delivery_zone']['#options']) == 1) {
      $form['search']['delivery_zone']['#required'] = FALSE;
    }
  }
  $form['search']['delivery_country']['#options'] = array('' => t('Please Select')) + $form['search']['delivery_country']['#options'];
  if (!$_GET['country']) {
    $form['search']['delivery_country']['#default_value'] = '';
  }
  $form['search']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Find Wish List'),
	  '#submit' => array('uc_wishlist_search_form_submit'),
  );
  
  return $form;
}

function uc_wishlist_search_form_submit($form, &$form_state) {
  if (empty($form_state['values']['first_name']) && empty($form_state['values']['last_name']) && empty($form_state['values']['city']) && empty($form_state['values']['delivery_country']) && empty($form_state['values']['delivery_zone'])) {
    drupal_goto('wishlist/search');
  }
  else {
    if ($form_state['values']['first_name']) {
      $query .= '&first_name=' . $form_state['values']['first_name'];
    }
    if ($form_state['values']['last_name']) {
      $query .= '&last_name=' . $form_state['values']['last_name'];
    }
    if ($form_state['values']['city']) {
      $query .= '&city=' . $form_state['values']['city'];
    }
    if ($form_state['values']['delivery_country']) {
      $query .= '&country=' . $form_state['values']['delivery_country'];
    }
    if ($form_state['values']['delivery_zone']) {
      $query .= '&zone=' . $form_state['values']['delivery_zone'];
    }
    drupal_goto('wishlist/search',$query);
  }
}

function uc_wishlist_create($nid = NULL, $qty = NULL, $data = NULL) {
  global $user;
  if (!$user->uid) {
    drupal_set_message('You must log in, or create an account, in order to create a Wish List.');
    drupal_goto('user/login','destination=wishlist/create');
    return;
  }
  $output = '';
  drupal_add_js('sites/all/themes/mightynest/limitchars.js');

  $wid = uc_wishlist_get_wid();
  
  if (!empty($wid)) {
    drupal_set_message('A Wish List already exists for your account.');
    drupal_goto('wishlist');
    return;
  }

  $output .= drupal_get_form('uc_wishlist_settings_form', NULL, FALSE);

  return $output;
}

