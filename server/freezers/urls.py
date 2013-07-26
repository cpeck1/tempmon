from django.conf.urls import patterns, url
from rest_framework.urlpatterns import format_suffix_patterns
from freezers import views
from django.conf.urls import include

urlpatterns = patterns('',
    url(r'^freezers/$', views.FreezerList.as_view()),
    url(r'^freezers/(?P<pk>[0-9]+)/$', views.FreezerDetail.as_view()),
)

urlpatterns = format_suffix_patterns(urlpatterns)
