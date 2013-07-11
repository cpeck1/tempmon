import os, sys
sys.path.append('/home/cpeck1/workspace/tempmon3/tempmon/server')
os.environ['DJANGO_SETTINGS_MODILE'] = 'PROJECT.settings'
import django.core.handlers.wsgi
application = django.core.handlers.wsgi.WSGIHandler()
