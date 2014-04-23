<?php
/*=========================================================================

  Program:   CDash - Cross-Platform Dashboard System
  Module:    $Id: config.php 3204 2012-02-15 08:06:00Z jjomier $
  Language:  PHP
  Date:      $Date: 2012-02-15 09:06:00 +0100 (mer., 15 févr. 2012) $
  Version:   $Revision: 3204 $

  Copyright (c) 2002 Kitware, Inc.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
/** WARNING: It's recommended to create a config.local.php file and leave
 * this file as is.
 * If creating the config.local.php from config.php make sure you DELETE
 * any text after the 'DO NOT EDIT AFTER THIS LINE' otherwise your
 * configuration file will be referencing each other. */

// This file is 'config.php', in the directory 'cdash', in the root.
// Therefore, the root of the CDash source tree on the web server is:
$CDASH_ROOT_DIR = str_replace("\\", "/", dirname(dirname(__FILE__)));

// Hostname of the database server
$CDASH_DB_HOST = getenv('DB_PORT_5432_TCP_ADDR');
// Login for database access
$CDASH_DB_LOGIN = 'cdash';
// Port for the database (leave empty to use default)
$CDASH_DB_PORT = getenv('DB_PORT_5432_TCP_PORT');
// Password for database access
$CDASH_DB_PASS = 'cdash';
// Name of the database
$CDASH_DB_NAME = 'cdash';
// Database type (empty means mysql)
$CDASH_DB_TYPE = 'pgsql';
// Turn this variable ON when CDash has been installed
// Prevents from running the install.php again
$CDASH_PRODUCTION_MODE = false;
$CDASH_TESTING_MODE = false;
$CDASH_TESTING_RENAME_LOGS = false;
// Should we use asynchronous submission
$CDASH_ASYNCHRONOUS_SUBMISSION = false;
// Main title and subtitle for the index page
$CDASH_MAININDEX_TITLE = 'CDash';
$CDASH_MAININDEX_SUBTITLE = 'Projects';
// Default from email
$CDASH_EMAILADMIN = 'admin@cdash.org';
$CDASH_EMAIL_FROM = 'admin@cdash.org';
$CDASH_EMAIL_REPLY = 'noreply@cdash.org';
// Should CDash only register valid emails
$CDASH_REGISTRATION_EMAIL_VERIFY = true;
// Duration of the cookie session (in seconds)
$CDASH_COOKIE_EXPIRATION_TIME='3600';
// Using HTTPS protocol to access CDash
$CDASH_USE_HTTPS ='0';
// Name of the server running CDash.
// Leave empty to use current name and default port.
$CDASH_SERVER_NAME = '';
$CDASH_SERVER_PORT = '';
// If the remote request should use localhost or the full name
// This variable should be set to 1 in most of the server configurations
$CDASH_CURL_REQUEST_LOCALHOST='1';
$CDASH_CURL_LOCALHOST_PREFIX='';
$CDASH_BASE_URL='';
// Define the location of the local directory
$CDASH_USE_LOCAL_DIRECTORY = '0';
// CSS file
$CDASH_CSS_FILE = 'cdash.css';
// Backup directory
$CDASH_BACKUP_DIRECTORY = $CDASH_ROOT_DIR.'/backup';
// Upload directory (absolute or relative)
$CDASH_UPLOAD_DIRECTORY = 'upload';
// The relative path from the CDash root dir to the $CDASH_UPLOAD_DIRECTORY (for downloading)
// http://<CDASH_URL>/<CDASH_DIR>/$CDASH_DOWNLOAD_RELATIVE_URL/<SHA-1>/<FILENAME>
// Note that this must be a relative path to the same directory specified by $CDASH_UPLOAD_DIRECTORY
$CDASH_DOWNLOAD_RELATIVE_URL = 'upload';
// Log file location
$CDASH_LOG_FILE = $CDASH_BACKUP_DIRECTORY."/cdash.log";
// Should normal user allowed to create projects
$CDASH_USER_CREATE_PROJECTS = false;
// Maximum size allocated for the logs
// CDash creates 10 files spanning the total size allocated
$CDASH_LOG_FILE_MAXSIZE_MB = 50;
// Using external authentication
$CDASH_EXTERNAL_AUTH = '0';
// Backup timeframe
$CDASH_BACKUP_TIMEFRAME = '48'; // 48 hours
// Request full email address to add new users
// instead of displaying a list
$CDASH_FULL_EMAIL_WHEN_ADDING_USER = '0';
// Warn about unregistered committers: default to '1' to keep
// the behavior the same as previous versions. Set to '0' to
// avoid "is not registered (or has no email)" warning messages.
$CDASH_WARN_ABOUT_UNREGISTERED_COMMITTERS = '0';
// Use getIPfromApache script to get IP addresses
// when using forwarding script
$CDASH_FORWARDING_IP='192.%'; // should be an SQL format
$CDASH_DEFAULT_IP_LOCATIONS = array();
// Use compression (default on)
$CDASH_USE_COMPRESSION='1';
// Use LDAP
$CDASH_USE_LDAP='0';
$CDASH_LDAP_HOSTNAME='localhost';
$CDASH_LDAP_BASEDN='ou=people,dc=organization,dc=com';
$CDASH_LDAP_PROTOCOL_VERSION='3';
// Additional LDAP query filters to restrict authorized user list
// Example: To restrict users to a specific Active Directory group:
// '(memberOf=cn=superCoolRescrictedGroup,cn=Users,dc=example,dc=com)'
$CDASH_LDAP_FILTER='';
// For authentication against Active Directory, set CDASH_LDAP_AUTHENTICATED to '1'
// CDASH_LDAP_OPT_REFERRALS to '0', and specify a bind DN and password
$CDASH_LDAP_OPT_REFERRALS='1';
$CDASH_LDAP_AUTHENTICATED='0';
$CDASH_LDAP_BIND_DN='cn=user,ou=people,dc=orgranization,dc=com';
$CDASH_LDAP_BIND_PASSWORD='password';
// Allow rememberme
$CDASH_ALLOW_LOGIN_COOKIE='1';
// Set to start the autoremoval on the first build of the day
$CDASH_AUTOREMOVE_BUILDS='0';
// Google Map API
$CDASH_GOOGLE_MAP_API_KEY = array();
$CDASH_GOOGLE_MAP_API_KEY['localhost'] = 'ABQIAAAAT7I3XxP5nXC2xZUbg5AhLhQlpUmSySBnNeRIYFXQdqJETZJpYBStoWsCJtLvtHDiIJzsxJ953H3rgg';
// Enable Google Analytics
$CDASH_DEFAULT_GOOGLE_ANALYTICS='';
// How long since the last submission before considering a project
// non active
$CDASH_ACTIVE_PROJECT_DAYS = '7'; // a week
// Use CDash to manage build submissions
// This feature is currently experimental
$CDASH_MANAGE_CLIENTS = '0';
// Define the git command
$CDASH_GIT_COMMAND = 'git';
// The default git directory where the bare repositories should be created
$CDASH_DEFAULT_GIT_DIRECTORY = 'git';
// Number of seconds to allow processing a single submission before resetting
$CDASH_SUBMISSION_PROCESSING_TIME_LIMIT = '450';
// Number of times to attempt processing a single submission before giving up
$CDASH_SUBMISSION_PROCESSING_MAX_ATTEMPTS = '5';
// Maximum per-project upload quota, in GB
$CDASH_MAX_UPLOAD_QUOTA = '10';
// Maximum size of large text fields, in php-strlen units, 0 for unlimited
$CDASH_LARGE_TEXT_LIMIT = '0';

