from django.forms import widgets
from rest_framework import serializers
from freezers.models import Freezer, LANGUAGE_CHOICES, STYLE_CHOICES
from datetime import datetime

from django.contrib.auth.models import User

class FreezerSerializer(serializers.Serializer):
    temperature = serializers.FloatField(required=True)
    last_updated = serializers.DateTimeField(required=False)
    def restore_object(self, attrs, instance=None):
        if instance:
            instance.temperature = attrs.get('temperature', 
                                             instance.temperature)
            instance.last_updated = datetime.now()
            return instance
        return Freezer(**attrs)
