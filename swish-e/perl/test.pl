#!/usr/local/bin/perl -w

    use strict;

=pod
    Test script for the SWISHE library

    please see perldoc README-PERL for more information
    $Id$
=cut

    # Import symbols from the SWISHE.pm module
    use SWISHE;


    # In this test we will use the same index twice
    # The results will seem odd since normally you would use
    # two different index files, but it demonstrates
    # how to process two index files.
    
    my $indexfilename1 = '../tests/test.index';
    my $indexfilename2 = $indexfilename1;

    die "Index file '$indexfilename1' not found!  Did you run make test from the top level directory?\n"
        unless -e $indexfilename1;

    my $indexfiles = $indexfilename1;        


    # To search for several indexes just put them together
    # Uncomment to test/demonstrate the use of two index files
    
    #my $indexfiles = "$indexfilename1 $indexfilename2";


    # First, open the index files. 
    # This reads in headers and prepares the index for searching
    # You can run more than one query once the index is opened.
    
    my $handle = SwishOpen( $indexfiles )
        or die "Failed to SwishOpen '$indexfiles'";


    # Get a few headers from the index files for display

    my @headers = qw/WordCharacters BeginCharacters EndCharacters/;
    push @headers, 'Indexed on';
  
    for ( @headers ) {
        print_header("Header '$_'");

        my @h =  SwishHeaderParameter( $handle, $_ );
        print "$_ for index 0 is $h[0]\n";
    }

    # Now, let's run a few queries...

    # Define a few searches

    my @searches = (
        {
            title   => 'Normal search',
            query   => 'test',
            props   => '',
            sort    => '',
            context => 1,   # Search the entire file
        },
        {
            title   => 'MetaTag search 1',
            query   => 'meta1=metatest1',
            props   => 'meta1 meta2 meta3',
            sort    => '',
            context => 1,   # Search the entire file
        },
        {
            title   => 'MetaTag search 2',
            query   => 'meta2=metatest2',
            props   => 'meta1 meta2 meta3',
            sort    => '',
            context => 1,   # Search the entire file
        },
        {
            title   => 'XML Search',
            query   => 'meta3=metatest3',
            props   => 'meta1 meta2 meta3',
            sort    => '',
            context => 1,   # Search the entire file
        },
        {
            title   => 'Phrase Search',
            query   => 'meta3="three little pigs"',
            props   => 'meta1 meta2 meta3',
            sort    => '',
            context => 1,   # Search the entire file
        },
        {
            title   => 'Advanced search',
            query   => 'test or meta1=m* or meta2=m* or meta3=m*',
            props   => 'meta1 meta2 meta3',
            sort    => '',
            context => 1,   # Search the entire file
        },
    );

    # Need an array in perl to deliver the above hash contents to swish in
    # the correct order
    my @settings = qw/query context props sort/;



    print_header("*** Now searching ****");
    print "Note that some META names have embedded newlines.\n";


    # Use an array for a hash slice when reading results.  See SwishNext below.
    my @labels = qw/
        rank
        file_name
        index_file_name
        last_modified_date
        title
        document_summary
        document_offset
        content_length
    /;


    for my $search ( @searches ) {
        print_header( "$search->{title} - Query: '$search->{query}'" );

        # Here's the actual query
        
        my $num_results = SwishSearch( $handle, @{$search}{ @settings } );

        print "# Number of results = $num_results\n\n";

        unless ( $num_results ) {
            print "No Results\n";

            my $error = SwishError( $handle );
            print "Error number: $error\n" if $error;

            next;
        }

        my %result;
        my @properties;

        while ( ( @result{ @labels }, @properties ) = SwishNext( $handle )) {
            print join( ' ',
                  @result{qw/ rank file_name index_file_name/},
                  qq[ "$result{title}" ],
                  qq["$result{document_summary}"],
                  $result{document_offset},
                  $result{content_length},
                  map{ qq["$_"] } @properties,
                  ),"\n";
        }
    }

    print_header('Other Functions');


    # Now, demonstrate the use of SwishStem to find the stem of a word.

    my @stemwords = qw/parking libaries library librarians money monies running runs is/;
    print "\nStemming:\n";
    print "    '$_' => '" . SwishStem( $_ ) . "'\n" for @stemwords;
    print "\n";

    
    # Grab the stop words from the header

    my @stopwords = SwishStopWords( $handle, $indexfilename1 );
    print 'Stopwords: ',
          ( @stopwords ? join(', ', @stopwords) : '** None **' ),
          "\n\n";


    # Let's see what words in the index begin with the letter "t".

    my $letter = 't';
    my @keywords = SwishWords( $handle, $indexfilename1, $letter);

    print "List of keywords that start with the letter '$letter':\n",
          join("\n", @keywords),
          "\n\n";



    # Free the memory.

    SwishClose( $handle );

sub print_header {
    print "\n", '-' x length( $_[0] ),"\n",
          $_[0],
          "\n", '-' x length( $_[0] ),"\n";
}


