<?php
header('Content-type: application/json');

$query = @$_GET['query'];
$country = @$_GET['country'];

if (!$query || strlen($query) > 50)
    die(json_encode([
        'status' => false,
        'result' => 'bad query'
    ]));

if (!$country || strlen($country) !== 2)
    die(json_encode([
        'status' => false,
        'result' => 'bad country'
    ]));

$prefix = 'https://itunes.apple.com/search?';
$url = $prefix .
    http_build_query([
        'term' => $query,
        'country' => $country,
        'media' => 'software',
        'limit' => 30,
        'lang' => 'en_us',
        'explicit' => 'Yes',
        'entity' => 'software,iPadSoftware,macSoftware'
    ]);

$ch = curl_init();
curl_setopt($ch, CURLOPT_URL, $url);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
$response = json_decode(curl_exec($ch));

if (!$response || $response->resultCount === 0)
    die(json_encode([
        'status' => false,
        'result' => 'no result'
    ]));

$result = [];

foreach ($response->results as $application) {
    $result[] = [
        'id' => $application->trackId,
        'title' => $application->trackCensoredName,
        'platform' => $application->kind === 'mac-software' ? 'Mac' : detectPlatform($application->supportedDevices),
        'developer' => $application->artistName,
        'genre' => $application->primaryGenreName, // or genres[] maybe?
        'icon' => $application->artworkUrl512,
        'price' => $application->formattedPrice
    ];
}

echo json_encode([
    'status' => true,
    'result' => $result
]);

function existsInArray(array $haystack, string $needle) {
    // Is Convert array to string and check with regex faster?

    foreach ($haystack as $element) {
        if (strstr($element, $needle))
            return true;
    }
}

function detectPlatform(array $supportedDevices) {
    $devices = [];

    if (existsInArray($supportedDevices, 'iPhone'))
        $devices[] = 'iPhone';

    if (existsInArray($supportedDevices, 'iPad'))
        $devices[] = 'iPad';

    if (existsInArray($supportedDevices, 'Mac'))
        $devices[] = 'Mac';

    if (count($devices) === 3)
        return 'Universal';
    else if (count($devices) === 1)
        return $devices[0];
    else
        return implode(' & ', $devices);
}
