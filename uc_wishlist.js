/**
 * @file
 * Small helper JS for the registry settings form address.
 */


/**
 * Apply the selected address to the appropriate fields in the cart form.
 */
function apply_address(type, address_str) {
  if (address_str == '0') {
    return;
  }

  eval('var address = ' + address_str + ';');

  $('#edit-' + type + '-first-name').val(address.first_name).trigger('change');
  $('#edit-' + type + '-last-name').val(address.last_name).trigger('change');
  $('#edit-' + type + '-phone').val(address.phone).trigger('change');
  $('#edit-' + type + '-company').val(address.company).trigger('change');
  $('#edit-' + type + '-street1').val(address.street1).trigger('change');
  $('#edit-' + type + '-street2').val(address.street2).trigger('change');
  $('#edit-' + type + '-city').val(address.city).trigger('change');
  $('#edit-' + type + '-postal-code').val(address.postal_code).trigger('change');

  if ($('#edit-' + type + '-country').val() != address.country) {
    $('#edit-' + type + '-country').val(address.country);
    try {
      uc_update_zone_select('#edit-' + type + '-country', address.zone);
    }
    catch (err) {}
  }

  $('#edit-' + type + '-zone').val(address.zone).trigger('change');
}

Drupal.behaviors.wishlist = function(context) {
  $("#delivery-pane input[name='panes[delivery][delivery_address_select]']").change(function() {
    if ($("#delivery-pane input[name='panes[delivery][delivery_address_select]']:checked").val() == 'wishlist_address') {
      $('#delivery-pane .address-pane-table').slideUp();
      $('#edit-panes-billing-copy-address-wrapper input').attr('checked', false);
      $('#edit-panes-billing-copy-address-wrapper').hide();
      $('#billing-pane .address-pane-table').slideDown();
      $('#edit-panes-quotes-quote-button').click();
    }
    else {
      $('#delivery-pane .address-pane-table').slideDown();
      $('#billing-pane .address-pane-table').slideDown();
      $('#edit-panes-billing-copy-address-wrapper').show();
      
      if ($("#delivery-pane input[name='panes[delivery][delivery_address_select]']:checked").val() == 'new_address') {
        $('#delivery-pane input.form-text').val('');
      }
      if ($("#delivery-pane input[name='panes[delivery][delivery_address_select]']:checked").val() == 'saved_address') {
        delivery_address_apply('delivery',$("#select-saved-address").val());
      }
    }
  });
  $("#select-saved-address").change(function() {
    delivery_address_apply('delivery',this.value);
  });
  $("select#edit-panes-billing-billing-address-select").change(function() {
    delivery_address_apply('billing',this.value);
  });
}

function delivery_address_apply(type,addr) {
  if (addr == '0') {
    return;
  }
  eval('var address = ' + addr + ';');
  var temp = type + '-' + type;
  $('#edit-panes-' + temp + '-first-name').val(address.first_name);
  $('#edit-panes-' + temp + '-last-name').val(address.last_name);
  $('#edit-panes-' + temp + '-phone').val(address.phone);
  $('#edit-panes-' + temp + '-company').val(address.company);
  $('#edit-panes-' + temp + '-street1').val(address.street1);
  $('#edit-panes-' + temp + '-street2').val(address.street2);
  $('#edit-panes-' + temp + '-city').val(address.city);
  $('#edit-panes-' + temp + '-postal-code').val(address.postal_code);

  if ($('#edit-panes-' + temp + '-country').val() != address.country) {
    $('#edit-panes-' + temp + '-country').val(address.country);
    try {
      uc_update_zone_select('edit-panes-' + temp + '-country', address.zone);
    }
    catch (err) { }
  }

  $('#edit-panes-' + temp + '-zone').val(address.zone);
}

$(document).ready(function() { 
  $("#delivery-pane input[name='panes[delivery][delivery_address_select]']").trigger('change');
});