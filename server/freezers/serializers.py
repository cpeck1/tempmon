from django.forms import widgets
from rest_framework import serializers
from freezers.models import Freezer
from datetime import datetime

from django.contrib.auth.models import User

class FreezerSerializer(serializers.Serializer):
    temperature = serializers.FloatField()
    status = serializers.CharField(max_length=100)
    last_updated = serializers.DateTimeField(required=False)

    def restore_object(self, attrs, instance=None):
        if instance:
            instance.temperature = attrs.get('temperature', 
                                             instance.temperature)
            instance.status = attrs.get('status', instance.status)
            instance.last_updated = datetime.now()
            return instance
        return Freezer(**attrs)
