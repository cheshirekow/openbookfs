#!/usr/bin/perl

my @extensions  = ("cpp","h","hpp");
my @directories = ("src");
my @filelist;


foreach $directory( @directories )
{
    print "searching $directory\n";
    foreach $extension( @extensions )
    {
        print "   for *.$extension\n";
        open INLIST, " find $directory -name '*.$extension' |";
        while(<INLIST>)
        {
            chomp;
            push(@filelist, $_);
        }
        close INLIST;
    }
}

$totallines     = 0;
$written        = 0;
$noncomment     = 0;

my $blockcomment    = 0;
my $comment         = 0;

print "parsing files\n";

foreach $file( @filelist )
{
    print "   $file\n";
    open INFILE, "./" . $file or die "failed to open" . $file . "\n";
    
    while(<INFILE>)
    {
        chomp;
        s/\s//g;
        
        $totallines++;
        
        $comment        = 0;
        if( /^(\/\/)/ )   {$comment        = 1;  }
        if( /(\/\*)/ )    {$blockcomment   = 1;  }
        if( /(\*\/)/ )    {$blockcomment   = 0;  }
        
        $noncomment++   unless( length($_) == 0 || $comment || $blockcomment );
        $written++      unless( length($_) == 0 );
    }
    
    close INFILE;
}

print "\n";
print "total:       " . $totallines     . "\n";
print "written:     " . $written        . "\n";
print "noncomment:  " . $noncomment     . "\n";

$dummy = <>;
