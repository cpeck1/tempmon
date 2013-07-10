from django.conf.urls.defaults import *

urlpatterns = patterns('',
                       url(r'^', include('freezers.urls')),
                       url(r'^', include('specifications.urls')),
)
