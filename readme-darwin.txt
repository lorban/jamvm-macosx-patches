* The OpenJDK6 available at http://www.openscg.com/se/openjdk/index.jsp can't run swing apps as it tries to
  load CoreFoundation libs from a non-main thread, see: http://openradar.appspot.com/7209349
  SoyLatte (http://landonf.bikemonkey.org/static/soylatte/) is older but works.
   -> not a JamVM bug

