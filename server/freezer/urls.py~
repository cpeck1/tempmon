from django.conf.urls import patterns, url
from rest_framework.urlpatterns import format_suffix_patterns
from freezer import views

urlpatterns = patterns('',
    url(r'^freezer/$', views.FreezerList.as_view()),
    url(r'^freezer/(?P<pk>[0-9]+)/$', views.FreezerDetail.as_view()),
)

urlpatterns = format_suffix_patterns(urlpatterns)
