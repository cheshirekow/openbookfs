#!/usr/bin/perl

#  Copyright (C) 2012 Josh Bialkowski (jbialk@mit.edu)
#
#  This file is part of openbook.
#
#  openbook is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  openbook is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with openbook.  If not, see <http://www.gnu.org/licenses/>.
#
#   @file   scripts/message_map.pl
#   @date   Apr 15, 2013
#   @author Josh Bialkowski <jbialk@mit.edu>
#   @brief  generates support for messages.h and messages.cpp, an enum
#           of message ID'S, a templates for ID->type and type->ID maps,
#           and an array of strings for printing the enum

use POSIX qw(strftime);
use File::Basename;
my $dirname = dirname(__FILE__);

# maps enum strings to class names
$messageMap = 
[
    ['QUIT'              ,'Quit'],
    ['PING'              ,'Ping'],
    ['PONG'              ,'Pong'],
    ['SET_DISPLAY_NAME'  ,'SetDisplayName'],
    ['SET_DATA_DIR'      ,'SetDataDir'],
    ['SET_LOCAL_SOCKET'  ,'SetLocalSocket'],
    ['SET_REMOTE_SOCKET' ,'SetRemoteSocket'],
    ['SET_CLIENT_SOCKET' ,'SetClientSocket'],
    ['SET_MAX_CONN'      ,'SetMaxConnections'],
    ['LOAD_CONFIG'       ,'LoadConfig'],
    ['SAVE_CONFIG'       ,'SaveConfig'], 
    ['ATTEMPT_CONNECT'   ,'AttemptConnection'],
    ['UI_REPLY'          ,'UserInterfaceReply'],
    ['LEADER_ELECT'      ,'LeaderElect'],
    ['DH_PARAMS'         ,'DiffieHellmanParams'],
    ['KEY_EXCHANGE'      ,'KeyExchange'],
    ['CEK'               ,'ContentKey'],
    ['AUTH_REQ'          ,'AuthRequest'],
    ['AUTH_CHALLENGE'    ,'AuthChallenge'],
    ['AUTH_SOLN'         ,'AuthSolution'],
    ['AUTH_RESULT'       ,'AuthResult'],
    ['SUBSCRIBE'         ,'Subscribe'],
    ['UNSUBSCRIBE'       ,'Unsubscribe'],
    ['NEW_VERSION'       ,'NewVersion'],
    ['REQUEST_FILE'      ,'RequestFile'],
    ['FILE_INFO'         ,'FileInfo'],
    ['FILE_CHUNK'        ,'FileChunk'],
    ['DIR_INFO'          ,'DirInfo'],
    ['DIR_CHUNK'         ,'DirChunk'],
    ['INVALID'           ,'Invalid'],
];


print "Enum String:\n";
print enum_string();
make_header("msg_gen","MessageId.h",enum_string());

print "Map String: \n";
print map_string();
make_header("msg_gen","MessageMap.h",map_string());

print "Name String: \n";
print name_string();
make_header("msg_gen","MessageStr.h","exter const char* g_msgStr[];");
#make_source("msg_gen","MessageStr.cpp",name_string());

print "Handler String: \n";
print handler_string();
make_header("msg_gen","MessageHandlerProto.h",handler_string());

# creates a header file and puts boiler plate stuff in it along with
# the argument 
sub make_header()
{
	my $relDir   = $_[0];
	my $fileName = $_[1];
	my $content  = $_[2];
	my $macro    = header_macro("$relDir/$fileName");
	my $fh;
	open ($fh, ">", "$dirname/../$relDir/$fileName" );
	print $fh copyright_string();
	print $fh filecomment_string("$relDir/$fileName");
	print $fh "\n\n#ifndef $macro\n#define $macro\n\n";
	print $fh $content;
	print $fh "\n\n#endif //< $macro\n\n";
	close($fh);
}

sub copyright_string()
{
	return <<'HERE'
/*
 *  Copyright (C) 2012 Josh Bialkowski (jbialk@mit.edu)
 *
 *  This file is part of openbook.
 *
 *  openbook is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  openbook is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with openbook.  If not, see <http://www.gnu.org/licenses/>.
 */
HERE


}

sub filecomment_string()
{
	my $date = strftime("%b %d, %Y", localtime);
	my $path = $_[0];
    return <<"HERE"
/**
 *  \@file   $path
 *
 *  \@date   $date
 *  \@author Josh Bialkowski (jbialk\@mit.edu)
 *  \@brief  
 */
HERE
;
}

sub header_macro()
{
	my $path = $_[0];
	$path =~ s/[\W]/_/g;   # replace non alpha chars with underscore
	$path = uc($path);     # make uppercase
	$path = "OPENBOOK_FS_" . $path;    # append prefix
	return $path
}

# returns a string in the form of:
#	enum MessageId
#	{
#	    MSG_QUIT,
#	    MSG_PING,
#	    MSG_PONG,
#	    ...
#	    MSG_INVALID,
#	    NUM_MSG = MSG_INVALID,
#	};
sub enum_string()
{
	my $result = "enum MessageId\n{\n";
	foreach $group ( @$messageMap )
	{
		my $key = $group->[0];
		$result .= sprintf("    MSG_%s,\n",$key);
	}
	$result .= "    NUM_MSG = MSG_INVALID,\n};\n\n";
	return $result;
}

# returns a string in the form of
#	MAP_MSG_TYPE(QUIT,              Quit)
#	MAP_MSG_TYPE(PING,              Ping)
#	MAP_MSG_TYPE(PONG,              Pong)
#   ..
#	MAP_MSG_TYPE(INVALID,           Invalid)
sub map_string()
{
	my $keyLen = max_key_length() + 1;
	my $format = '%' . sprintf('%d',$keyLen) . 's';
	my $result = "";
	
    foreach $group ( @$messageMap )
    {
    	my $key    = $group->[0];
    	my $value  = $group->[1];
    	my $keyStr = sprintf( $format, $key . ',' ); 
        $result .= sprintf("MAP_MSG_TYPE(%s %s)\n",$keyStr,$value);
    }
    return $result . "\n";
}

# returns a string in the form of
#    const char* g_msgStr[] =
#    {
#        "QUIT",
#        "PING",
#        "PONG",
#        ...
#        "INVALID_ID"
#    };
sub name_string()
{
	my $result = 'const char* g_msgIdStr[] ='. "\n{\n";
    foreach $group ( @$messageMap )
    {
    	my $key = $group->[0];
        $result .= sprintf('    "%s",'."\n",$key);
    }
    $result .= "};\n\n";
    return $result;
}

# returns a string in the form of
#        void handleMessage( messages::Quit*         msg );
#        void handleMessage( messages::Ping*         msg );
#        void handleMessage( messages::Pong*         msg );
#        ...
#        void handleMessage( messages::Invalid*      msg );
sub handler_string()
{
	my $keyLen = max_value_length() + 1;
    my $format = '%-' . sprintf('%d',$keyLen) . 's';
    my $result = "";
    
    foreach $group ( @$messageMap )
    {
        my $key    = $group->[1];
        my $keyStr = sprintf( $format, $key . '*' ); 
        $result .= sprintf("void handleMessage( messages::%s msg);\n",$keyStr);
    }
    return $result . "\n";
}

# returns the max key length
sub max_key_length()
{
	my $maxLen = 0;
    foreach $group ( @$messageMap )
    {
    	my $key = $group->[0];
    	if( length($key) > $maxLen )
    	{
    	    $maxLen = length($key);
    	}
    }

    return $maxLen;
}

# returns the max value length
sub max_value_length()
{
	my $maxLen = 0;
    foreach $group ( @$messageMap )
    {
        my $key = $group->[1];
        if( length($key) > $maxLen )
        {
            $maxLen = length($key);
        }
    }

    return $maxLen;
}




