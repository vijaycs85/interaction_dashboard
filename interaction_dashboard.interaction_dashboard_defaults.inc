<?php
/**
 * @file
 *   Defualt interaction services implementation.
 */

/**
 * Enter description here ...
 * @return multitype:number NULL 
 */
function interaction_dashboard_get_od_pair() {
  $service = messagequeue_service_load('get_od_pair');
  $od = $service->invoke(array());
  if (!empty($od)) {
    $count = count($od->ListODOutput->ODReferenceDataOutputs->ODReferenceDataOutput);
    return array(
        'status' => INTERACTION_DASHBOARD_SUCCESS,
        'message' => t('Loaded %number OD pairs successfully!!!', array('%number' => $count)),
    );
  }
  return interaction_dashboard_get_default();
}

/**
 * Enter description here ...
 * @return multitype:number NULL 
 */
function interaction_dashboard_get_booking_horizon() {
  $booking_horizon = origin_destination_get_booking_horizon_data();
  if (!empty($booking_horizon)) {
    $count = count($booking_horizon->ListBookingHorizonOutput->BookingHorizons->BookingHorizon);
    return array(
      'status' => INTERACTION_DASHBOARD_SUCCESS,
      'message' => t('Loaded %number booking horizon data successfully!!!', array('%number' => $count)),
    );
  }
  return interaction_dashboard_get_default();
}

/**
 * Enter description here ...
 * @return multitype:number NULL 
 */
function interaction_dashboard_get_list_credit_card_fees() {
  $fees = itinerary_get_credit_card_fees();
  if ($fees->getFees()) {
    $fees_details = $fees->getFees();
    $cards = implode(', ', array_keys($fees_details['charges']));
    return array(
      'status' => INTERACTION_DASHBOARD_SUCCESS,
      'message' => t('Retrieved surcharges for  %cards card(s) successfully!!!', array('%cards' => $cards)),
    );
  }
  return interaction_dashboard_get_default();
}

/**
 * Enter description here ...
 * @return multitype:number NULL 
 */
function interaction_dashboard_get_booking_quotation() {
  $train_search = _interaction_dashboard_get_train_search();
  $outward_proposals = $train_search->getProposalsForDay(Journey::$TYPE_OUTWARD);
  if (!empty($outward_proposals)) {
    return array(
      'status' => INTERACTION_DASHBOARD_SUCCESS,
      'message' => t('Retrieved %journey_type proposals successfully!!!', array('%journey_type' => Journey::$TYPE_OUTWARD)),
    );
  }
  return interaction_dashboard_get_default();
}

/**
 * Enter description here ...
 */
function interaction_dashboard_cancel_reservation(){
    $make_reservation = _interaction_dashboad_get_reservation();
  // Check status and set return value.
  if ($pnr = $make_reservation->getPNR()) {
    $cancel_reservation = messagequeue_service_load('cancel_reservation');
    $cancel_reservation->pnr_reference = $pnr;
    $cancel_reservation->invoke();
    if ($status = $cancel_reservation->getStatus()) {
      return array(
        'status' => INTERACTION_DASHBOARD_SUCCESS,
        'message' => t('cancellation service working. Cancellation status of PNR @pnr is @status', array('@pnr' => $pnr, '@status' => $status)),
      );
    }
  }
  return interaction_dashboard_get_default();
}

/**
 * Enter description here ...
 */
function interaction_dashboard_make_reservation(){
  $make_reservation = _interaction_dashboad_get_reservation();
  // Check status and set return value.
  if ($pnr = $make_reservation->getPNR()) {
    return array(
        'status' => INTERACTION_DASHBOARD_SUCCESS,
        'message' => t('Reservation service working successfully. PNR is @pnr', array('@pnr' => $pnr)),
    );
  }
  return interaction_dashboard_get_default();
}

/**
 * Enter description here ...
 */
function interaction_dashboard_exchange_outbound() {
  $passengers = array(
  array('first_name' => 'F1', 'last_name' => 'L1', 'passenger_id' => 1),
  array('first_name' => 'F2', 'last_name' => 'L2', 'passenger_id' => 2),
  );
  //$id, $type, $origin_code, $destination_code, DateTime $departure_date, DateTime $arrival_date, $duration, TrainTicketProductWrapper $ticket = NULL, Price $price = NULL
  $od = array(
    'origin_code' => 'GBSPX',
    'destination_code' => 'FRPNO'
    );
    $travel = LoadTravel::create();
    $travel->pnr = 'TKPLPB';
    $travel->stub = 1;
    $travel->invoke();
    print_r($travel->isFlexible());exit;

    $service =  GetExchangeProposal::create();
    $service->departure_date = $travel->getDepartureTimestamp();
    $service->return_date = $travel->getArrivalTimestamp();
    $service->origin_code = $travel->getOrigin();
    $service->destination_code = $travel->getDestination();
    $service->pnr_reference = 'TKPLPB';
    foreach ($passengers as $passenger) {
      $service->addPassenger($passenger['passenger_id'], $passenger['first_name'], $passenger['last_name']);
    }
    // $origin_destination = entity_load('origin_destination', FALSE, array('origin_code' => '7015400', 'destination_code' => '8727100'));
    // $od = array_pop($origin_destination);
    // $service->setOD($od);
    $service->setOldDepartureDateFromTimestamp($travel->getDepartureTimestamp());
    //$service->setJourneyId(1);

    $service->setDepartureDateFromTimestamp(time() + 20000);
    $service->setReturnDateFromTimestamp(time() + 40000);

    $service->invoke();
    //$service->getPp
    exit;
}

/**
 * Enter description here ...
 */
function interaction_dashboard_exchange_inbound() {
  $passengers = array(
  array('first_name' => 'F1', 'last_name' => 'L1', 'passenger_id' => 1),
  array('first_name' => 'F2', 'last_name' => 'L2', 'passenger_id' => 2),
  );
  $od = array(
    'originCode' => 'GBSPX',
    'destinationCode' => 'FRPNO',
  );
  $service =  GetExchangeProposal::create();
  $service->setPnrReference('ABCDEF');
  foreach ($passengers as $passenger) {
    $service->addPassenger($passenger['passenger_id'], $passenger['first_name'], $passenger['last_name']);
  }
  $origin_destination = entity_load('origin_destination', FALSE, array('origin_code' => '7015400', 'destination_code' => '8727100'));
  $od = array_pop($origin_destination);
  $service->setOD($od);
  $service->setOperationType(1);
  $service->setJourneyId(1);
  $service->journey_type = 'Inward';
  $service->setDepartureDateFromTimestamp(time() + 10000);
  $service->setReturnDateFromTimestamp(time() + 20000);
  $service->invoke();
}

/**
 * Enter description here ...
 */
function interaction_dashboard_validate_proposal() {
  /*
   $start = time() + 10001;
   $end = time() + 20000;
   $segment_details['segment_data'] = array(
   array(
   'endDate' => date('d/m/Y', $end),
   'endTime' => date('H:i', $end),
   'startDate' => date('d/m/Y', $start),
   'startTime' => date('H:i', $start),
   'segmentId' => 1,
   'trainNumber' => '9000',
   'carrierCode' => 1,
   'depart' => $end,
   'arrival' => $start,
   'originCode' => 'GBSPX',
   'destinationCode' => 'FRPNO',
   'marketingTrainNumber' => 5000,
   'marketingCarrierCode' => 1,
   'passengerFare' => array(
   array('classOfService' => 'AF', 'fareCode'=>'BVSFLASB', 'passengerTypeCode'=>'PT01AD','passengerId'=>1),
   array('classOfService' => 'AF', 'fareCode'=>'BVSFLASB', 'passengerTypeCode'=>'PT01AD','passengerId'=>2),
   )
   )
   );
   $journey_details = array(
   array(
   'originCode' => 'GBSPX',
   'destinationCode' => 'FRPNO',
   'depart' => $end,
   'arrival' => $start,
   'journeyId' => 1,
   ),
   );
   $params = array(
   'pnrReference' => 'AAAAAA',
   'passengers' => array(
   array('firstName' => 'F1', 'lastName' => 'L1', 'passengerId' => 1),
   array('firstName' => 'F2', 'lastName' => 'L2', 'passengerId' => 2),
   ),
   'inboundDateTime' => time()+ 1000,
   'outboundDateTime' => time(),
   'depart' => time() + 10001,
   'arrival' => time() + 10000,
   'from' => 'GBSPX',
   'to' => 'FRPNO',
   'inbound' => $segment_details,
   'outbound' => $segment_details,
   'journeyDetails' => $journey_details,
   );
   */
  $service =  ValidateProposal::create();
  $service->pnr_reference = 'TPWJOD';
  //$id, $type, $origin_code, $destination_code, DateTime $departure_date, DateTime $arrival_date, $duration, TrainTicketProductWrapper $ticket = NULL, Price $price = NULL) {
  $departure_date = new DateTime();
  $departure_date->setTimeStamp(strtotime('10/05/2012 06:24:00'));
  $arrival_date = new DateTime();
  $arrival_date->setTimeStamp(strtotime('10/05/2012 13:25:00'));
  $origin_code = 7054660;
  $destination_code = 8775100;
  $journey = new Journey(1, Journey::$TYPE_OUTWARD, $origin_code, $destination_code, $departure_date, $arrival_date, 7*60+1);
  //$id, $type, $title, $first_name, $last_name, $age, $wheel_chair = FALSE, $companion = FALSE, $carriage_number = NULL, $seat_number = NULL, $meal_code = NULL, Price $price = NULL) {
  $passengers[] = new Passenger('1.1', Passenger::$TYPE_ADULT, 'Mr', 'JOHN', 'BOSCO', Passenger::$PASSENGER_AGE[Passenger::$TYPE_ADULT]);
  $passengers[] = new Passenger('2.1', Passenger::$TYPE_ADULT, 'Mrs', 'ROSE', 'JOHN', Passenger::$PASSENGER_AGE[Passenger::$TYPE_ADULT]);
  $passengers[] = new Passenger('3.1', Passenger::$TYPE_ADULT, 'Mr', 'YOUTH', 'JOHN', Passenger::$PASSENGER_AGE[Passenger::$TYPE_ADULT]);
  //($id, $type, $origin_code, $destination_code, DateTime $origin_code, DateTime $arrival_date, $duration, CarrierEntityWrapper $carrier)
  $segments[] = new Segment(1, Journey::$TYPE_OUTWARD, $origin_code, $destination_code, $departure_date, $arrival_date, 150);
  $segments[] = new Segment(2, Journey::$TYPE_OUTWARD, $origin_code, $destination_code, $departure_date, $arrival_date, 150);

  // Passenger $passenger, $passenger_type_code, $class_of_service, $fare_code, $seat_preference_code = NULL, $toc_fare_code = NULL, $toc_fare_description = NULL
  $passenger = new Passenger('1.1', Passenger::$TYPE_ADULT, 'Mr', 'JOHN', 'BOSCO', Passenger::$PASSENGER_AGE[Passenger::$TYPE_ADULT]);
  $passenger_fares[] = new PassengerFare($passenger, 'PT00AD', 'BF', 'BFBASB');
  $passenger_fares[] = new PassengerFare($passenger, 'PT00AD', 'BF', 'BFBASB');

  $service->addSelectedJourney(Journey::$TYPE_OUTWARD, $segments, $passenger_fares);
  $service->addSelectedJourney(Journey::$TYPE_INWARD, $segments, $passenger_fares);

  $service->addJourney($journey);
  foreach ($passengers as $passenger) {
    $service->addPassenger($passenger);
  }


  $result = $service->invoke($params);
  //print_r($result);exit;
}

/**
 * Enter description here ...
 */
function interaction_dashboard_authorize_card() {

  $params = array(
  // @todo purchase_desc needs to be figured out how it will be assembled.
          'purchase_desc' => 'Ticket purchase', //$payment['description'],
          'currency' => 'GBP', //$payment['amount']['currency_code'],
          'amount' => '125', //commerce_currency_amount_to_decimal($payment['amount']['amount'], $payment['amount']['currency_code']),
          'card' => array(
            'street_address1' => 'Address 1',
            'street_address2' => NULL,
            'postcode' => 'AB1 2CD',
            'issue_number' => NULL,
            'start_date' => NULL,
            'expiry_date' => '08/12',
            'pan' => '1000171234567896',
            'cv2' => '123',
  ),
  );

  $service =  messagequeue_service_load('authorize_card');
  $service->setHeaders(array('authorizeRetryCount' => 1));
  $result = $service->invoke($params);
  //print_r($result);exit;
}

/**
 * Enter description here ...
 */
function interaction_dashboard_authorize_card_cc_token() {

  $params = array(
  // @todo purchase_desc needs to be figured out how it will be assembled.
          'purchase_desc' => 'Ticket purchase', //$payment['description'],
          'currency' => 'GBP', //$payment['amount']['currency_code'],
          'amount' => '125', //commerce_currency_amount_to_decimal($payment['amount']['amount'], $payment['amount']['currency_code']),
          'rateId' => '3900900010002469',
          'action' => 'opt_in',
          'card' => array(
            'street_address1' => 'Address 1',
            'street_address2' => NULL,
            'postcode' => 'AB1 2CD',
            'issue_number' => NULL,
            'start_date' => NULL,
            'expiry_date' => '08/12',
            'pan' => 'FACBE350B7FB506E56E21524679FB07BEB83C157',
            'cv2' => '123',
  ),

  );

  $service =  messagequeue_service_load('authorize_card_cc_token');
  $service->setHeaders(array('authorizeRetryCount' => 1));
  $result = $service->invoke($params);
  //print_r($result);exit;
}

/**
 * Enter description here ...
 */
function interaction_dashboard_modify_passenger_data() {
  list($make_reservation, $train_search) = _interaction_dashboad_get_reservation(TRUE);
  if ($pnr = $make_reservation->getPNR()) {

    // @TODO: Invoke SBE to update traveller details.
    $modify_passenger_data = messagequeue_service_load('modify_passenger_data');
    $modify_passenger_data->setPnr($pnr);

    // Save traveller details in multistep cache so other panes can use it.
    $selected_proposal = $train_search->getSelectedProposal(Journey::$TYPE_OUTWARD);
    $passengers = $selected_proposal->getPassengers();
    $segments = $selected_proposal->getSegments();
    foreach($segments as $segment) {
      foreach ($passengers as $passenger) {
        // Update passenger information
        $passenger->title = UserEntityWrapper::getTitleName('Mr');
        $passenger->first_name = 'firstname' . $passenger->id;
        $passenger->last_name = 'lastname' . $passenger->id;
        $passenger->eft_number = 308381;
        //$passenger->meal_code = '';
        $modify_passenger_data->addPassenger($passenger->id, $passenger);
      }
    }
    $modify_passenger_data->invoke();
    if ($modify_passenger_data->getStatus()) {
      return interaction_dashboard_get_default(t('Passenger details modified and status is @status', array('@status' => $modify_passenger_data->getStatus())));
    }
  }
  else {
      return interaction_dashboard_get_default(t('Make reservation service is falling'));
  }
  return interaction_dashboard_get_default();
}

/**
 * Enter description here ...
 */
function interaction_dashboard_finalize_exchange() {
  $start = time() + 10001;
  $end = time() + 20000;
  $passengers = array(
    'ADULT' => array(
  array('first_name' => 'F1', 'last_name' => 'L1', 'passenger_id' => '1', 'membership' => '1001', 'wheelchair' => FALSE, 'title' => 'Mr'),
  array('first_name' => 'F2', 'last_name' => 'L2', 'passenger_id' => '2', 'membership' => '1002', 'wheelchair' => TRUE, 'title' => 'Mr'),
  ),
  );
  $segment_details['segment_data'] = array(
  array(
        'endDate' => date('d/m/Y', $end),
        'endTime' => date('H:i', $end),
        'startDate' => date('d/m/Y', $start),
        'startTime' => date('H:i', $start),
        'segmentId' => 1,
        'trainNumber' => '9000',
        'carrierCode' => 1,
        'depart' => $end,
        'arrival' => $start,
        'originCode' => 'GBSPX',
        'destinationCode' => 'FRPNO',
        'marketingTrainNumber' => 5000,
        'marketingCarrierCode' => 1,
        'passengerFare' => array(
  array('classOfService' => 'AF', 'fareCode'=>'BVSFLASB', 'passengerTypeCode'=>'PT01AD','passengerId'=>1),
  array('classOfService' => 'AF', 'fareCode'=>'BVSFLASB', 'passengerTypeCode'=>'PT01AD','passengerId'=>2),
  )
  )
  );
  $journey_details = array(
      'originCode' => 'GBSPX',
      'destinationCode' => 'FRPNO',
      'depart' => $end,
      'arrival' => $start,
      'journeyId' => 1,
  );
  $params = array(
  'pnr_reference' => 'AAAAAA',
  'passengers' => array(
  array('firstName' => 'F1', 'lastName' => 'L1', 'passengerId' => 1),
  array('firstName' => 'F2', 'lastName' => 'L2', 'passengerId' => 2),
  ),
  'inboundDateTime' => time()+ 1000,
  'outboundDateTime' => time(),
  'depart' => time() + 10001,
  'arrival' => time() + 10000,
  'from' => 'GBSPX',
  'to' => 'FRPNO',
  'inbound' => $segment_details,
  'outbound' => $segment_details, 
  'passengers' => $passengers,
  'journeyDetails' => $journey_details,
    'products' => array (
  array('reference' => 'Product 1', 'type' => 1),
  ),
    'card' => array('amount' =>'120', 'currency'=>'GBP', 'timestamp' =>time()),
    'contact' => array ('emailAddress' => 'test@test.com', 'phoneNumber' => '+44 121 112 1212'),
    'pan' => 1,
  );
  $service =  messagequeue_service_load('finalize_exchange');
  $service->setHeaders(array('operation' => 'exchange'));
  // $service->stub = TRUE;
  $result = $service->invoke($params);
  //print_r($result);exit;


}

/**
 * Enter description here ...
 * @return multitype:number NULL 
 */
function interaction_dashboard_get_booking_quotation_by_period() {
  $origin_destination = entity_load('origin_destination', FALSE, array('origin_code' => 'GBLON', 'destination_code' => 'FRPAR'));
  if (empty($origin_destination)) {
    watchdog('interaction_dashboard', t('Error while testing %origin to %destination', array('%origin' => 'GBLON', '%destination' => 'FRPAR',)));
  }
  $origin_destination = array_pop($origin_destination);
  $search_criteria = array(
    'origin_destination' => $origin_destination,
    'depart' => strtotime('+7 days 6 am'),
    'adult' => '2',
    'senior' => '1',
    'youth' => '0',
    'child' => '0',
    'return' => strtotime('+37 days 11 am'),
  );
  $train_search = new TrainSearch($search_criteria['origin_destination'], $search_criteria['depart'], $search_criteria['adult'], $search_criteria['senior'], $search_criteria['youth'], $search_criteria['child'], $search_criteria['return']);

  $start_date = $train_search->getPeriodStartDate();
  $proposals = $train_search->getProposalsForPeriod(Journey::$TYPE_OUTWARD, $start_date);
  if (!empty($proposals)) {
    return array(
      'status' => INTERACTION_DASHBOARD_SUCCESS,
      'message' => t('Retrieved %journey_type period proposals successfully!!!', array('%journey_type' => Journey::$TYPE_OUTWARD)),
    );
  }
  return interaction_dashboard_get_default();
}

/**
 * Enter description here ...
 */
function interaction_dashboard_manage_user() {
  $service =  ManageCustomer::create();// print_r($service);
  $result = $service->addMemberContact('testing28@mail.com', 'I', 'H');
  $service->setDefaults();
  $service->invoke();
  return $service->getCustomerNumber();
}

/**
 * Enter description here ...
 */
function interaction_dashboard_get_hotel_rate_rules() {
  $service =  GetHotelRateRules::create();
  $result = $service->setReservationToken('adsfadsf');
  $result = $service->setGeneralProperties('adsfasdf');
  $service->invoke();
  //return $service->getCustomerNumber();
}

/**
 * Enter description here ...
 */
function interaction_dashboard_checkout() {
  $checkout = Checkout::create();
  $checkout->stub = TRUE;
  $contact = array(
    'address_line_1' => 'Address 1',
    'town' => 'London',
    'country_2' => 'Address 1',
    'first_name' => 'John',
    'last_name' => 'Bosco',
    'std_code' => '121',
    'email' => 'asdaf@mail.com',
    'phone' => '7778889999',
    'post_code' => '12121',
    'title' => 'Mr',
  );
  $checkout->setContactDetails($contact);
  $checkout->setCardFee(120);
  $checkout->setPnrReference('ABCDEF');
  $checkout->setDatacashReference('Payment123');
  $checkout->setPromocodeReference('PROMO123');
  $checkout->invoke();
  echo $checkout->bookingEventLinkID();
  echo $checkout->getTransactionState();exit;
}

/**
 * Enter description here ...
 */
function interaction_dashboard_load_travel() {
    $make_reservation = _interaction_dashboad_get_reservation();
  // Check status and set return value.
  if ($pnr = $make_reservation->getPNR()) {
    $travel = LoadTravel::create();
    $travel->setPnr($pnr);
    $travel->invoke();
    if ($travel->getOrigin()) {
      return array(
        'status' => INTERACTION_DASHBOARD_SUCCESS,
        'message' => t('Retrieved travel details for given PNR @pnr successfully', array('@pnr' => $pnr)),
      );
    }
  }
  else {
    interaction_dashboard_get_default(t(('Make reservation is falling')));
  }
  return interaction_dashboard_get_default();
}

/**
 * Enter description here ...
 */
function interaction_dashboard_refund_proposal() {
  $service = GetRefundProposal::create();
  $service->pnr_reference = 'TPWJOD';
  //$id, $type, $origin_code, $destination_code, DateTime $departure_date, DateTime $arrival_date, $duration, TrainTicketProductWrapper $ticket = NULL, Price $price = NULL) {
  $departure_date = new DateTime();
  $departure_date->setTimeStamp(strtotime('10/05/2012 06:24:00'));
  $arrival_date = new DateTime();
  $arrival_date->setTimeStamp(strtotime('10/05/2012 13:25:00'));
  $journey = new Journey(1, Journey::$TYPE_OUTWARD, 7054660, 8775100, $departure_date, $arrival_date, 7*60+1);
  //$id, $type, $title, $first_name, $last_name, $age, $wheel_chair = FALSE, $companion = FALSE, $carriage_number = NULL, $seat_number = NULL, $meal_code = NULL, Price $price = NULL) {
  $passengers[] = new Passenger('1.1', Passenger::$TYPE_ADULT, 'Mr', 'JOHN', 'BOSCO', Passenger::$PASSENGER_AGE[Passenger::$TYPE_ADULT]);
  $passengers[] = new Passenger('2.1', Passenger::$TYPE_ADULT, 'Mrs', 'ROSE', 'JOHN', Passenger::$PASSENGER_AGE[Passenger::$TYPE_ADULT]);
  $passengers[] = new Passenger('3.1', Passenger::$TYPE_ADULT, 'Mr', 'YOUTH', 'JOHN', Passenger::$PASSENGER_AGE[Passenger::$TYPE_ADULT]);

  $service->addJourney($journey);
  foreach ($passengers as $passenger) {
    $service->addPassenger($passenger);
  }
  $service->invoke();
}

/**
 * Enter description here ...
 */
function interaction_dashboard_finalize_refund() {
  $service = FinalizeRefund::create();
  $service->pnr_reference = 'TPWJOD';
  //$id, $type, $origin_code, $destination_code, DateTime $departure_date, DateTime $arrival_date, $duration, TrainTicketProductWrapper $ticket = NULL, Price $price = NULL) {
  $departure_date = new DateTime();
  $departure_date->setTimeStamp(strtotime('10/05/2012 06:24:00'));
  $arrival_date = new DateTime();
  $arrival_date->setTimeStamp(strtotime('10/05/2012 13:25:00'));
  $journey = new Journey(1, Journey::$TYPE_OUTWARD, 7054660, 8775100, $departure_date, $arrival_date, 7*60+1);
  //$id, $type, $title, $first_name, $last_name, $age, $wheel_chair = FALSE, $companion = FALSE, $carriage_number = NULL, $seat_number = NULL, $meal_code = NULL, Price $price = NULL) {
  $passengers[] = new Passenger('1.1', Passenger::$TYPE_ADULT, 'Mr', 'JOHN', 'BOSCO', Passenger::$PASSENGER_AGE[Passenger::$TYPE_ADULT]);
  $passengers[] = new Passenger('2.1', Passenger::$TYPE_ADULT, 'Mrs', 'ROSE', 'JOHN', Passenger::$PASSENGER_AGE[Passenger::$TYPE_ADULT]);
  $passengers[] = new Passenger('3.1', Passenger::$TYPE_ADULT, 'Mr', 'YOUTH', 'JOHN', Passenger::$PASSENGER_AGE[Passenger::$TYPE_ADULT]);
  // $scheme, $surcharge, $expiry_date, $token = NULL, psp_transaction_id
  $service->addCreditCardTransaction(new CreditCardTransaction('VISA', 0, '08/13', NULL, 'transaction_id'));
  $service->addCreditCardTransaction(new CreditCardTransaction('MASTER', 0, '08/13', NULL, 'transaction_id'));
  // $id, $amount, $expire, $psp_transaction_id
  // $service->addVoucherTransaction(new HistoryVoucher('VO12', 10.12, time(), 'transaction_id'));

  $service->addJourney($journey);
  foreach ($passengers as $passenger) {
    $service->addPassenger($passenger);
  }
  $service->invoke();
}

/**
 * Enter description here ...
 * @return multitype:number NULL 
 */
function interaction_dashboard_find_address() {
  $paf_service = AddressFinder::create();
  $paf_service->stub = TRUE;
  $paf_service->propertyDetail = '';
  $paf_service->invoke();
  $address = $paf_service->getAddress();
  if (isset($address)) {
    return array(
        'status' => INTERACTION_DASHBOARD_SUCCESS,
        'message' => t('Address service working successfully!!!')
    );
  }
  return interaction_dashboard_get_default();

}

/**
 * Enter description here ...
 * @return multitype:number NULL 
 */
function interaction_dashboard_get_hotel_availability() {
  $hotel_search = interaction_dashboard_call_get_hotel_availability();
  $hotels = $hotel_search->getHotels();
  list($start_date, $end_date) = explode(',', $hotel_search->getDateRange());
  if(isset($hotels)) {
    $hotel_details = array(
      '%number' => $hotel_search->getHotelCount(),
      '%start_date' => $start_date,
      '%end_date' => $end_date,
      '%city' => $hotel_search->getScope(),
    );
    return array(
      'status' => INTERACTION_DASHBOARD_SUCCESS,
      'message' => t('found %number hotel(s) to stay from %start_date to %end_date in %city', $hotel_details),
    );
  }
  return interaction_dashboard_get_default();
}

/**
 * Enter description here ...
 * @return Ambiguous
 */
function interaction_dashboard_call_get_hotel_availability(){
  $scope_type = 'City';
  $scope =array('London','London','GB');
  $room_occupancy = array('NumAdults' => 2, 'NumChildren' => array(1), 'NumSeniors' => 1);
  $date_range = array(
    'start' => date('Y-m-d', time()+(2*24*60*60)),
    'end' => date('Y-m-d', time()+(4*24*60*60)),
  );
  $class_of_service = NULL;
  $display_options = array();
  $service_path = 'BP';
  $hotel_name = NULL;
  $hotel_rating = NULL;
  $hotel_amenities = NULL;
  // Load hotel availability object
  $hotel_search = messagequeue_service_load('get_hotel_availability');
  // Set the scope for the search
  $hotel_search->setScope($scope_type, $scope);
  // Set the RoomOccupancy expected is an array('NumAdults' => 1, 'NumChildren' => array(5, 10), 'NumSeniors' => 0)

  $hotel_search->setRoomOccupancy($room_occupancy);
  $hotel_search->setDateRange($date_range['start'], $date_range['end']);
  $hotel_search->setDisplayOptions($display_options);
  if (!is_null($class_of_service)) $hotel_search->setClassOfService($class_of_service);
  // @todo Change to pass service path in AS or BP
  $hotel_search->setServicePath($service_path);
  if (!is_null($hotel_name)) $hotel_search->setNameFilter($hotel_name);
  if (!is_null($hotel_rating)) $hotel_search->setStarRating($hotel_rating);
  if (!is_null($hotel_amenities)) $hotel_search->setAmenities($hotel_amenities);
  $hotel_search->invoke();
  return $hotel_search;
}
/**
 * Enter description here ...
 */
function interaction_dashboard_generate_pah() {

}

/**
 * Enter description here ...
 * @return multitype:number NULL
 */
function interaction_dashboard_create_lite_customer() {
  $email = uniqid().'.servic_test@localhost.com';
  if ($iloyal_customer_id = UserEntityWrapper::createLiteAccount($email)) {
    return array(
      'status' => INTERACTION_DASHBOARD_SUCCESS,
      'message' => t('lite user created(customer id:@customer_id) with email id: @mail', array('@customer_id' => $iloyal_customer_id,'@mail' => $email)),
    );
  }
  return interaction_dashboard_get_default();
}

/**
 * Enter description here ...
 * @return multitype:number NULL 
 */
function interaction_dashboard_retrieve_customer() {
  $email = uniqid().'.servic_test@localhost.com';
  if ($iloyal_customer_id = UserEntityWrapper::createLiteAccount($email)) {
    $user_serivce = RetrieveCustomer::create();
    $user_serivce->customer_number = $iloyal_customer_id;
    $user_serivce->invoke();
    $response_email  =(string) current($user_serivce->getResponse()->xpath('//memberContactInfos/emailAddress'));
    if ($response_email == $email) {
      return array(
        'status' => INTERACTION_DASHBOARD_SUCCESS,
        'message' => t('Retrived user details from iLoyal for customer-id:@customer_id', array('@customer_id' => $iloyal_customer_id)),
      );
    }
  }
  return interaction_dashboard_get_default();
}

/**
 *
 */
function interaction_dashboard_manage_customer() {
  $email = uniqid().'.servic_test@localhost.com';
  if ($iloyal_customer_id = UserEntityWrapper::createLiteAccount($email)) {
    $account = user_load_by_mail($email);
    $wrapper = extended_entity_wrapper('user', $account, array(), 'UserEntityWrapper');
    $wrapper->field_user_iloyal_customer_no->set($iloyal_customer_id);
    $wrapper->field_user_comm_pref->set($form_state['values']['main_preference'] ? 'Y' : 'N');
    $wrapper->field_user_comm_promotion_offers->set($form_state['values']['promotion_offers'] ? 'Y' : 'N');
    $wrapper->field_user_comm_product_updates->set($form_state['values']['product_updates'] ? 'Y' : 'N');
    $wrapper->field_user_comm_information->set($form_state['values']['information'] ? 'Y' : 'N');
    $wrapper->field_user_comm_whats_happening->set($form_state['values']['whats_happening'] ? 'Y' : 'N');
    $wrapper->field_user_comm_country->set($form_state['values']['country_residence']);
    if ($wrapper->updateILoyal('preferences')) {
      $account = user_save($account);
      $replace = array('!uid' => $wrapper->value()->uid, '!iloyal' => $wrapper->field_user_iloyal_customer_no->value());
      return array(
        'message' => t('Updated iLoyal preferences from Drupal user for uid !uid, iLoyal customer number !iloyal', $replace),
        'status' => INTERACTION_DASHBOARD_SUCCESS,
      );
    }
  }
  return interaction_dashboard_get_default();
}

function _interaction_dashboard_get_train_search() {
  $origin_destination = entity_load('origin_destination', FALSE, array('origin_code' => 'GBLON', 'destination_code' => 'FRPAR'));
  if (empty($origin_destination)) {
    watchdog('interaction_dashboard', t('Error while testing %origin to %destination', array('%origin' => 'GBLON', '%destination' => 'FRPAR',)));
  }
  $origin_destination = array_pop($origin_destination);
  $search_criteria = array(
    'origin_destination' => $origin_destination,
    'depart' => strtotime('+2 days 6 am'),
    'adult' => '2',
    'senior' => '1',
    'youth' => '0',
    'child' => '0',
    'return' => strtotime('+5 days 11 am'),
  );
  $train_search = new TrainSearch($search_criteria['origin_destination'], $search_criteria['depart'], $search_criteria['adult'], $search_criteria['senior'], $search_criteria['youth'], $search_criteria['child'], $search_criteria['return']);
  return $train_search;
}

function _interaction_dashboad_get_reservation($get_search = FALSE) {
  $train_search = _interaction_dashboard_get_train_search();
  _interaction_dashboard_set_proposal($train_search);

  $make_reservation = messagequeue_service_load('make_reservation');
  $make_reservation->addSelectedProposal($train_search->getSelectedProposal(Journey::$TYPE_OUTWARD));
  $make_reservation->invoke();
  if ($get_search) {
    return array ($make_reservation, $train_search);
  }
  return $make_reservation;
}
function _interaction_dashboard_set_proposal(&$train_search) {
  $proposals = $train_search->getProposalsForDay(Journey::$TYPE_OUTWARD);
  foreach ($proposals as $journey_id => $journey) {
    foreach ($journey as $proposal) {
      $ticket_class = $proposal->getTicketClass();
      $ticket_flexibility = $proposal->getTicketFlexibility();
    }
  }
  $train_search->selectProposal($journey_id, $ticket_class, $ticket_flexibility);
  return $train_search;
}