# teltonika_speedtest_task
An internship task for Teltonika IoT Academy. A network speed testing tool that is able to conduct download/upload speed tests, as well as locate hosts and find the optimal host for a specified region.

## Dependencies

- **libcurl**
- **getopt**
- **cJSON**

## Flags

| Flag                | Usage                     | Meaning |
|---------------------|---------------------------|---------|
| -d                  |   -d                      | Conduct download speed test. |
| -u                  |   -u                      | Conduct upload speed test. |
| -s                  |   -s                      | Report the IP of the user. |
| -l                  |   -l                      | Report the location (country, city) of a host. If specified with no server directives or with the --user flag, reports the location of the user, if specified with a server directive, reports the location of all hosts. |
|--path               | --path \<filepath\>       | Load hosts from JSON. |
|--host               | --host \<address\>        | Load a single host by its address. |
|                     | --timeout \<sec\>         | Total operational timeout for downloads and uploads. |
|--city               | --city \<city\>           | Work with hosts only from this city. |
|--country            | --country \<country\>     | Work with hosts only from this country. |
|--best_city          | --best_city \<city\>      | Get the optimal host in this city. Required and functional only when executing a joint test. |
|--best_country       | --best_country \<country\>| Get the optimal host in this country. Required and functional only when executing a joint test. |
|--user               | --user                    | Get the location of the user if the -l flag is specified. |
|--disableSSL         | --disableSSL              | Disable SSL verification. |
|--joint              | --joint                   | Take such actions: Test each host's download and upload speeds; get the optimal host in the specified location; and report the IP, as well as the location, of the user. |


## JSON input

A JSON file's structure must conform to such requirements in order to be accepted by the program:
- The root element must be an array
- Every object must have the property `host`. Additionally, they may accomodate the following properties (note that missing cities and countries of servers are not acquired during runtime to accomodate the execution of filters):
    - `country`: string
    - `city`: string
    - `provider`: string
    - `id`: number