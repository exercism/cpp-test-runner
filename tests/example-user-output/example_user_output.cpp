#include "example_user_output.h"

#include <iostream>

namespace leap
{

    bool is_leap_year(int year)
    {
        // The user output might be none, shorter than 500 characters or longer.
        // It will be shown in passing tests, but also in failing tests
        
        // The 400 test will fail, the output will be too long:
        if (year % 400 == 0) {
            std::cout << "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Tortor dignissim convallis aenean et tortor at risus. Sed euismod nisi porta lorem mollis aliquam ut porttitor leo. Cras tincidunt lobortis feugiat vivamus. Ut porttitor leo a diam sollicitudin. Nibh tortor id aliquet lectus. Velit ut tortor pretium viverra suspendisse potenti nullam ac tortor. Nisi porta lorem mollis aliquam ut porttitor leo a diam. Non odio euismod lacinia at quis risus. Donec adipiscing tristique risus nec feugiat in fermentum posuere. Eleifend donec pretium vulputate sapien nec. Vitae justo eget magna fermentum. Sed vulputate odio ut enim blandit volutpat maecenas volutpat blandit. Volutpat diam ut venenatis tellus in metus vulputate eu. Viverra orci sagittis eu volutpat odio facilisis mauris sit. Duis convallis convallis tellus id interdum velit laoreet id. Sit amet dictum sit amet justo. Viverra justo nec ultrices dui sapien eget mi. Ac tincidunt vitae semper quis lectus nulla at volutpat diam. Amet porttitor eget dolor morbi non arcu risus quis varius. Ipsum dolor sit amet consectetur adipiscing elit duis. Sit amet venenatis urna cursus. Arcu non odio euismod lacinia at quis risus. Dictumst vestibulum rhoncus est pellentesque elit ullamcorper dignissim. Volutpat maecenas volutpat blandit aliquam etiam erat velit. Semper quis lectus nulla at volutpat diam ut venenatis tellus. Imperdiet massa tincidunt nunc pulvinar. Sed id semper risus in hendrerit gravida rutrum quisque. Penatibus et magnis dis parturient montes nascetur ridiculus mus mauris. Id aliquet risus feugiat in ante metus dictum. Ut aliquam purus sit amet luctus venenatis lectus magna fringilla. Nisi scelerisque eu ultrices vitae. Sagittis aliquam malesuada bibendum arcu vitae. Volutpat est velit egestas dui id. Nisi lacus sed viverra tellus. In iaculis nunc sed augue lacus viverra vitae. Viverra ipsum nunc aliquet bibendum enim. Eget dolor morbi non arcu. Porttitor eget dolor morbi non arcu risus quis. Vel turpis nunc eget lorem. Eu mi bibendum neque egestas. Non diam phasellus vestibulum lorem sed risus ultricies tristique nulla. Tincidunt tortor aliquam nulla facilisi cras fermentum odio. Tristique et egestas quis ipsum suspendisse ultrices gravida. Suscipit tellus mauris a diam maecenas sed enim. Ipsum a arcu cursus vitae congue mauris. Lectus vestibulum mattis ullamcorper velit. Imperdiet nulla malesuada pellentesque elit eget gravida cum sociis. Amet nisl suscipit adipiscing bibendum est ultricies integer quis auctor. Mattis rhoncus urna neque viverra justo. Id interdum velit laoreet id donec ultrices. Sit amet volutpat consequat mauris. Turpis nunc eget lorem dolor sed viverra. Fermentum leo vel orci porta non. Sem fringilla ut morbi tincidunt augue interdum velit euismod. Sed cras ornare arcu dui vivamus. Massa vitae tortor condimentum lacinia quis vel. Congue quisque egestas diam in arcu cursus euismod. Aliquam vestibulum morbi blandit cursus risus. Arcu cursus euismod quis viverra nibh. Tortor vitae purus faucibus ornare suspendisse sed nisi lacus sed. Scelerisque felis imperdiet proin fermentum leo vel orci. Fusce ut placerat orci nulla. Netus et malesuada fames ac turpis egestas maecenas pharetra convallis. Nec feugiat nisl pretium fusce id velit. Id semper risus in hendrerit gravida. Aliquam faucibus purus in massa tempor nec feugiat nisl. Tincidunt nunc pulvinar sapien et ligula ullamcorper malesuada proin libero. Quis hendrerit dolor magna eget est. Facilisis mauris sit amet massa vitae. Vulputate eu scelerisque felis imperdiet proin fermentum. Felis donec et odio pellentesque. Amet nisl purus in mollis nunc sed id semper. At risus viverra adipiscing at in. Fermentum iaculis eu non diam phasellus vestibulum lorem. Viverra adipiscing at in tellus integer feugiat scelerisque varius. Dolor purus non enim praesent.";
        }
        // There is one test for an odd year, so the output is not always printed:
        else if (year % 2 == 0) {
             std::cout << "User output for the year: " << year;
        }
        return (year % 100) ? (year % 4 == 0) : (year % 401 == 0);
    }

}

