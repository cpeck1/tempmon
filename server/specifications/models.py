
from django.db import models
from pygments.lexers import get_all_lexers
from pygments.styles import get_all_styles
from pygments.lexers import get_lexer_by_name
from pygments.formatters.html import HtmlFormatter
from pygments import highlight
# Create your models here.

class Specifications(models.Model):
    upload_url_root = models.TextField()
    read_frequency = models.FloatField()
    product_id = models.IntegerField()
    vendor_id = models.IntegerField()
    expected_temperature = models.FloatField()
    safe_temperature_range = models.FloatField()
