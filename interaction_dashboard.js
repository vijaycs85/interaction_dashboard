(function ($) {
  Drupal.behaviors.interactionDashboard = {
    attach: function(context, settings) {
      // Features management form
      $('table.features:not(.processed)').each(function() {
        $(this).addClass('processed');

        // Check the overridden status of each feature
        Drupal.interactionDashboard.checkStatus();

        // Add some nicer row hilighting when checkboxes change values
        $('input', this).bind('change', function() {
          if (!$(this).attr('checked')) {
            $(this).parents('tr').removeClass('enabled').addClass('disabled');
          }
          else {
            $(this).parents('tr').addClass('enabled').removeClass('disabled');
          }
        });
      });

      // Export form component selector
      $('form.features-export-form select.features-select-components:not(.processed)').each(function() {
        $(this)
          .addClass('processed')
          .change(function() {
            var target = $(this).val();
            $('div.features-select').hide();
            $('div.features-select-' + target).show();
            return false;
        });
      });

      // Export form machine-readable JS
      $('.feature-name:not(.processed)').each(function() {
        $('.feature-name')
          .addClass('processed')
          .after(' <small class="feature-module-name-suffix">&nbsp;</small>');
        if ($('.feature-module-name').val() === $('.feature-name').val().toLowerCase().replace(/[^a-z0-9]+/g, '_').replace(/_+/g, '_') || $('.feature-module-name').val() === '') {
          $('.feature-module-name').parents('.form-item').hide();
          $('.feature-name').bind('keyup change', function() {
            var machine = $(this).val().toLowerCase().replace(/[^a-z0-9]+/g, '_').replace(/_+/g, '_');
            if (machine !== '_' && machine !== '') {
              $('.feature-module-name').val(machine);
              $('.feature-module-name-suffix').empty().append(' Machine name: ' + machine + ' [').append($('<a href="#">'+ Drupal.t('Edit') +'</a>').click(function() {
                $('.feature-module-name').parents('.form-item').show();
                $('.feature-module-name-suffix').hide();
                $('.feature-name').unbind('keyup');
                return false;
              })).append(']');
            }
            else {
              $('.feature-module-name').val(machine);
              $('.feature-module-name-suffix').text('');
            }
          });
          $('.feature-name').keyup();
        }
      });
      $('table.features tbody tr a.admin-reload').click(function(e) {
        e.preventDefault();
          var elem = $(this).parent().parent();
          // hide all element and show only loading.
          $(elem).find('.state span, .state a').hide();
          $(elem).find('.admin-loading').show();
          $(elem).find('.status').html('Fetching ...');
          $(elem).addClass('processed');
          var uri = $(elem).find('a.admin-check').attr('href');
          if (uri) {
            $.get(uri, [], function(data) {
              $(elem).find('.admin-loading').hide();
              $(elem).find('.admin-reload').show();
              if (data.status == null) {
                 var data = {"status":2, "Message":"Service doesn't return proper status. try again!!!" };
              }
              switch (data.status) {
                case 1:
                  $(elem).find('.admin-default').show();
                  $(elem).find('.status').html(data.message);
                  break;
                default:
                  $(elem).find('.admin-overridden').show();
                  $(elem).find('.status').html(data.message);
                  break;
              }
              // Drupal.interactionDashboard.checkItemStatus();
            }, 'json');
          }
        });
    }
  }


  Drupal.interactionDashboard = {
    'checkStatus': function() {
      $('table.features tbody tr').not('.processed').filter(':first').each(function() {
        var elem = $(this);
        $(elem).addClass('processed');
        var uri = $(this).find('a.admin-check').attr('href');
        if (uri) {
          $.get(uri, [], function(data) {
            $(elem).find('.admin-loading').hide();
            $(elem).find('.admin-reload').show();
            if (data == null) {
              var data = {"status":2, "Message":"Service doesn't return proper status. try again!!!" };
            }
            switch (data.status) {
              case 1:
                $(elem).find('.admin-default').show();
                $(elem).find('.status').html(data.message);
                break;
              default:
                $(elem).find('.admin-overridden').show();
                $(elem).find('.status').html(data.message);
                break;
            }
            Drupal.interactionDashboard.checkStatus();
          }, 'json');
        }
        else {
            Drupal.interactionDashboard.checkStatus();
          }
      });
    },
  };
})(jQuery);


