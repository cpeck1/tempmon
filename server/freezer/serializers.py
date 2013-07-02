from django.forms import widgets
from django.db import models
from rest_framework import serializers
from freezer.models import Freezer, LANGUAGE_CHOICES, STYLE_CHOICES

class FreezerSerializer(serializers.ModelSerializer):
    class Meta:
        model = Freezer
        fields = ('number', 'last_update', 'temperature')
