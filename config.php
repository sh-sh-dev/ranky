<?php
include_once 'vendor/autoload.php';

$countryCodes = ['AR', 'AU', 'AT', 'BE', 'BR', 'CA', 'CN', 'DK', 'FI', 'FR', 'DE', 'HK', 'IN', 'IE', 'IL', 'IT', 'JP', 'KR', 'MX', 'NL', 'NZ', 'NO', 'PL', 'PT', 'RU', 'SA', 'SG', 'ZA', 'ES', 'SE', 'CH', 'TR', 'UA', 'AE', 'GB', 'US'];
$genresMac = [36, 6000, 6001, 6002, 6003, 6004, 6005, 6006, 6007, 6008, 6009, 6010, 6011, 6012, 6013, 6014, 6015, 6016, 6017, 6020, 6024, 6026, 6027];

$genres = [
    // General Genres

    36 => 'Overall',
    6000 => 'Business',
    6001 => 'Weather',
    6002 => 'Utilities',
    6003 => 'Travel',
    6004 => 'Sports',
    6005 => 'Social Networking',
    6006 => 'Reference',
    6007 => 'Productivity',
    6008 => 'Photo & Video',
    6009 => 'News',
    6010 => 'Navigation',
    6011 => 'Music',
    6012 => 'Lifestyle',
    6013 => 'Health & Fitness',
    6014 => 'Games',
    6015 => 'Finance',
    6016 => 'Entertainment',
    6017 => 'Education',
    6018 => 'Books',
    6020 => 'Medical',
    6021 => 'Magazines & Newspapers',
    6022 => 'Catalogs',
    6023 => 'Food & Drink',
    6024 => 'Shopping',
    6025 => 'Stickers',
    6026 => 'Developer Tools',
    6027 => 'Graphics & Design',

    // Games Subgenres

    7001 => 'Games > Action',
    7002 => 'Games > Adventure',
    7003 => 'Games > Casual',
    7004 => 'Games > Board',
    7005 => 'Games > Card',
    7006 => 'Games > Casino',
    7007 => 'Games > Dice',
    7008 => 'Games > Educational',
    7009 => 'Games > Family',
    7011 => 'Games > Music',
    7012 => 'Games > Puzzle',
    7013 => 'Games > Racing',
    7014 => 'Games > Role Playing',
    7015 => 'Games > Simulation',
    7016 => 'Games > Sports',
    7017 => 'Games > Strategy',
    7018 => 'Games > Trivia',
    7019 => 'Games > Word',

    // News & Magazines Subgenres

//    13001 => 'Magazines & Newspapers > News & Politics',
//    13002 => 'Magazines & Newspapers > Fashion & Style',
//    13003 => 'Magazines & Newspapers > Home & Garden',
//    13004 => 'Magazines & Newspapers > Outdoors & Nature',
//    13005 => 'Magazines & Newspapers > Sports & Leisure',
//    13006 => 'Magazines & Newspapers > Automotive',
//    13007 => 'Magazines & Newspapers > Arts & Photography',
//    13008 => 'Magazines & Newspapers > Brides & Weddings',
//    13009 => 'Magazines & Newspapers > Business & Investing',
//    13010 => 'Magazines & Newspapers > Children\'s Magazines',
//    13011 => 'Magazines & Newspapers > Magazines & Newspapers > Computers & Internet',
//    13012 => 'Magazines & Newspapers > Cooking, Food & Drink',
//    13013 => 'Magazines & Newspapers > Crafts & Hobbies',
//    13014 => 'Magazines & Newspapers > Electronics & Audio',
//    13015 => 'Magazines & Newspapers > Entertainment',
//    13017 => 'Magazines & Newspapers > Health, Mind & Body',
//    13018 => 'Magazines & Newspapers > History',
//    13019 => 'Magazines & Newspapers > Literary Magazines & Journals',
//    13020 => 'Magazines & Newspapers > Men\'s Interest',
//    13021 => 'Magazines & Newspapers > Movies & Music',
//    13023 => 'Magazines & Newspapers > Parenting & Family',
//    13024 => 'Magazines & Newspapers > Pets',
//    13025 => 'Magazines & Newspapers > Professional & Trade',
//    13026 => 'Magazines & Newspapers > Regional News',
//    13027 => 'Magazines & Newspapers > Science',
//    13028 => 'Magazines & Newspapers > Teens',
//    13029 => 'Magazines & Newspapers > Travel & Regional',
//    13030 => 'Magazines & Newspapers > Women\'s Interest',
];

$chartTypes = [
    'FreeAppsV2', // iPhone - Free
    'PaidApplications', // iPhone - Paid
    'AppsByRevenue', // iPhone - Grossing

    'FreeIPadApplications', // iPad - Free
    'PaidIPadApplications', // iPad - Paid
    'IPadAppsByRevenue', // iPad - Grossing

    'FreeMacAppsV2', // Mac - Free
//    'PaidMacApps', // Mac - Paid
//    'MacAppsByRevenue', // Mac - Grossing
];

$notificationsLink = 'https://domain.com/bin.php?type=ranky-fail';

$globalHeaders = [
    'Cache-Control: no-cache',
    'User-Agent: RankUp Agent/2.0'
];

$db = [
    'host' => 'localhost',
    'user' => 'mysql',
    'password' => 'pass',
    'name' => 'ranky'
];

$db = new mysqli($db['host'], $db['user'], $db['password'], $db['name']);
