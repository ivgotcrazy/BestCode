# -*- coding: utf-8 -*-
# Generated by Django 1.10 on 2017-05-01 11:35
from __future__ import unicode_literals

from django.db import migrations, models
import django.db.models.deletion


class Migration(migrations.Migration):

    initial = True

    dependencies = [
    ]

    operations = [
        migrations.CreateModel(
            name='Activity',
            fields=[
                ('activity_id', models.AutoField(primary_key=True, serialize=False)),
                ('name', models.CharField(default='最佳代码', max_length=64)),
                ('desc', models.TextField(max_length=4096)),
                ('result', models.TextField(max_length=4096)),
                ('photo', models.FileField(upload_to='./activities/static/activities')),
                ('start_date', models.DateTimeField(verbose_name='Start Time')),
                ('browse_times', models.IntegerField(verbose_name='Browse Times')),
            ],
        ),
        migrations.CreateModel(
            name='ActivityPlan',
            fields=[
                ('activity_plan_id', models.AutoField(primary_key=True, serialize=False)),
                ('plan_item_name', models.CharField(max_length=64)),
                ('plan_item_start', models.DateTimeField(verbose_name='Start time')),
                ('plan_item_end', models.DateTimeField(verbose_name='End time')),
                ('activity', models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, to='activities.Activity')),
            ],
        ),
        migrations.CreateModel(
            name='ActivityState',
            fields=[
                ('state_id', models.AutoField(primary_key=True, serialize=False)),
                ('state_name', models.CharField(max_length=64)),
            ],
        ),
        migrations.AddField(
            model_name='activity',
            name='state',
            field=models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, to='activities.ActivityState'),
        ),
    ]
