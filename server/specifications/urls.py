from django.conf.urls import patterns, url
from rest_framework.urlpatterns import format_suffix_patterns
from specifications import views
from django.conf.urls import include

urlpatterns = patterns('',
    url(r'^specifications/$', views.SpecificationsDetail.as_view()),
)

urlpatterns = format_suffix_patterns(urlpatterns)
